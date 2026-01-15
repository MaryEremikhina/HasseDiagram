#ifndef AUTOLABA_DRAW_H
#define AUTOLABA_DRAW_H

#include <algorithm>
#include <map>
#include <vector>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "HasseBuilder.h"
#include "AminoAcids.h"

struct DrawVertex {
    int index = 0;
    std::string string;
    float x = 0.0f;
    float y = 0.0f;
};

// разделение вершин на уровни для удобства красивой визуализации
inline std::map<int, std::vector<int>> levelIndex(const std::vector<HasseBuilder::Edge> &edges) {
    std::map<int, std::vector<int>> result;
    std::vector<int> used;
    for (const auto & edge : edges) {
        auto find1 = std::find(used.begin(), used.end(), edge.first);
        if (find1 == used.end()) {
            result[0].push_back(edge.first);
            used.push_back(edge.first);
        }
        auto find2 = std::find(used.begin(), used.end(), edge.second);
        if (find2 == used.end()) {
            int x = edge.first;
            auto it = std::find_if(result.begin(), result.end(),
                [x](const auto& kv) {
                    const std::vector<int>& v = kv.second;
                    return std::find(v.begin(), v.end(), x) != v.end();
            });
            if (it != result.end()) {
                result[it->first + 1].push_back(edge.second);
                used.push_back(edge.second);
            }
        }
    }
    return result;
}
// определение структуры DrawVertex для каждой вершины диаграммы Хассе
inline std::vector<DrawVertex> VerticesFromHasse(const std::vector<Element>& elements, const std::vector<HasseBuilder::Edge> &edges) {
    std::vector<DrawVertex> vertices;
    std::map<int, std::vector<int>> levels = levelIndex(edges);
    for (const auto& pair : levels) {
        for (int i = 0; i < pair.second.size(); i++) {
            DrawVertex vertex;
            vertex.index = pair.second[i];
            vertex.string = elements[pair.second[i]].ToString();
            vertex.y = -1.0f + (0.2f + static_cast<float>(pair.first) * 0.2f) * 2.0f;
            vertex.x = ((static_cast<float>(i) + 1.0f) / (static_cast<float>(pair.second.size()) + 1.0f)) * 2.0f - 1.0f;
            vertices.push_back(vertex);
        }
    }
    return vertices;
}

// отрисовка кругов для вершин диаграммы
inline void drawCircle(const float cx, const float cy, const float r, const int segments = 24) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); // центр

    for (int i = 0; i <= segments; ++i) {
        float a = 2.0f * 3.1415926f * static_cast<float>(i) / static_cast<float>(segments);
        float x = cx + r * std::cos(a);
        float y = cy + r * std::sin(a);
        glVertex2f(x, y);
    }
    glEnd();
}
// отрисовка текста для подписи вершин
inline void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; ++c) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); // выбрать шрифт поменьше
    }
}
inline int getTextWidth(const char* text, void* font) {
    int width = 0;
    for (const char* c = text; *c != '\0'; ++c)
        width += glutBitmapWidth(font, *c);
    return width;
}
inline void drawTextCentered(float x, float y, const char* text, void* font = GLUT_BITMAP_TIMES_ROMAN_24) {
    int w = getTextWidth(text, font);
    float scaleX = 1.0f / 600.0f;
    float cx = x - (w * scaleX) * 0.5f;
    glRasterPos2f(cx, y);
    for (const char* c = text; *c != '\0'; ++c)
        glutBitmapCharacter(font, *c);
}

// построение таблицы выравниваний
inline std::vector<std::string> GetTable(const std::vector<HasseBuilder::Edge> &edges, const std::vector<DrawVertex>& vertices) {
    std::vector<std::string> result;
    for (const auto& e : edges) {
        std::string elem;
        auto it1 = std::find_if(vertices.begin(), vertices.end(),
                [e](const DrawVertex& dv) { return dv.index == e.first; });
        auto it2 = std::find_if(vertices.begin(), vertices.end(),
            [e](const DrawVertex& dv) { return dv.index == e.second; });
        if (it1 != vertices.end() && it2 != vertices.end()) {
            elem = (*it1).string + " -> " + (*it2).string + " : ";
            std::pair<std::string, std::string> Equally = traceBack(DP((*it1).string, (*it2).string), (*it1).string, (*it2).string);
            elem += Equally.first + "|" + Equally.second;
        }
        result.push_back(elem);
    }
    return result;
}
// отрисовка таблицы для выравниваний
inline void drawAlignmentTable(const std::vector<std::string>& lines) {
    float x = -0.95f;
    float y = 0.95f;
    float dy = 0.04f;

    glColor3f(0.f, 0.f, 0.f);

    for (const auto& s : lines) {
        drawText(x, y, s.c_str());
        y -= dy;
    }
}

// сохранение графа на компьютер
inline void saveScreenshot(GLFWwindow* window, const char* filename) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    std::vector<unsigned char> pixels(width * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // OpenGL хранит снизу вверх
    std::vector<unsigned char> flipped(width * height * 3);
    for (int y = 0; y < height; ++y) {
        std::memcpy(
            &flipped[y * width * 3],
            &pixels[(height - 1 - y) * width * 3],
            width * 3
        );
    }
    stbi_write_png(filename, width, height, 3, flipped.data(), width * 3);
}

// вырисовка диаграммы Хассе
inline int DrawHasse(const std::vector<DrawVertex>& vertices, const std::vector<HasseBuilder::Edge> &edges) {
    if (!glfwInit())
        return -1;
    GLFWwindow* window = glfwCreateWindow(600, 600, "HasseDiagram", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(0.f, 0.f, 0.f);
        glBegin(GL_LINES);
        for (const auto& e : edges) {
            auto it1 = std::find_if(vertices.begin(), vertices.end(),
                [e](const DrawVertex& dv) { return dv.index == e.first; });
            auto it2 = std::find_if(vertices.begin(), vertices.end(),
                [e](const DrawVertex& dv) { return dv.index == e.second; });
            if (it1 != vertices.end() && it2 != vertices.end()) {
                const DrawVertex& parent = *it1;
                const DrawVertex& child  = *it2;
                glVertex2f(parent.x, parent.y);
                glVertex2f(child.x,  child.y);
            }
        }
        glEnd();

        for (const auto & vertice : vertices) {
            glColor3f(1.0f, 0.753f, 0.796f);
            drawCircle(vertice.x, vertice.y, 0.1f);
            glColor3f(0.f, 0.f, 0.f);
            drawTextCentered(vertice.x, vertice.y - 0.15f, vertice.string.c_str());
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
        saveScreenshot(window, "screenshot.png");
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
// вырисовка диаграммы Хассе для биоинформатики
inline int DrawHasseBio(const std::vector<DrawVertex>& vertices, const std::vector<HasseBuilder::Edge> &edges) {
    if (!glfwInit())
        return -1;
    GLFWwindow* window = glfwCreateWindow(600, 600, "HasseDiagramBio", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // отрисовка ребер
        glColor3f(0.f, 0.f, 0.f);
        glBegin(GL_LINES);
        for (const auto& e : edges) {
            auto it1 = std::find_if(vertices.begin(), vertices.end(),
                [e](const DrawVertex& dv) { return dv.index == e.first; });
            auto it2 = std::find_if(vertices.begin(), vertices.end(),
                [e](const DrawVertex& dv) { return dv.index == e.second; });
            if (it1 != vertices.end() && it2 != vertices.end()) {
                const DrawVertex& parent = *it1;
                const DrawVertex& child  = *it2;
                glVertex2f(parent.x, parent.y);
                glVertex2f(child.x,  child.y);
            }
        }
        glEnd();

        // отрисовка score для последовательностей
        for (const auto& e : edges) {
            auto it1 = std::find_if(vertices.begin(), vertices.end(),
                [e](const DrawVertex& dv) { return dv.index == e.first; });
            auto it2 = std::find_if(vertices.begin(), vertices.end(),
                [e](const DrawVertex& dv) { return dv.index == e.second; });

            if (it1 != vertices.end() && it2 != vertices.end()) {
                const DrawVertex& parent = *it1;
                const DrawVertex& child  = *it2;
                // отрисовка score
                float dx = child.x - parent.x;
                float dy = child.y - parent.y;
                float len = std::sqrt(dx*dx + dy*dy);
                float nx = -dy / len;
                float ny =  dx / len;
                float mx = (parent.x + child.x) * 0.5f;
                float my = (parent.y + child.y) * 0.5f;

                float offset = 0.05f; // регулируем размер смещения
                std::string label = std::to_string(Score(parent.string, child.string));
                drawTextCentered(mx + nx * offset, my + ny * offset, label.c_str());
                // отрисовка выравнивания
                std::vector<std::string> table = GetTable(edges, vertices);
                drawAlignmentTable(table);
            }
        }

        // отрисовка вершин с названиями
        for (const auto & vertice : vertices) {
            glColor3f(1.0f, 0.753f, 0.796f);
            drawCircle(vertice.x, vertice.y, 0.1f);
            glColor3f(0.f, 0.f, 0.f);
            drawTextCentered(vertice.x, vertice.y - 0.15f, vertice.string.c_str());
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
        saveScreenshot(window, "screenshot.png");
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

#endif //AUTOLABA_DRAW_H