#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "Element.h"
#include "Rules.h"
#include "HasseBuilder.h"
#include "Input.h"
#include "Draw.h"
#include "AminoAcids.h"

// отдельно выводить вершины, которые ни с чем не связаны - СДЕЛАНО!!!
// масштабирование вершин при увеличении их количества - СДЕЛАНО!!!
// поиск экстремальных характеристик - СДЕЛАНО!!!
// упорядочивать не по правилам, а по парам элементов (ввести либо по правилу, либо по парам) - СДЕЛАНО!!!

// привести тесты с большим количеством данных
// находить супремум среди подмножеств диаграммы Хассе ??? (пока забиваем)
// ввести возможность работы над любыми типами данных
// ЖЕЛАТЕЛЬНО!!! упорядочивать не по правилам, а по парам элементов (ввести либо по правилу, либо по парам)

/*
 * просто передаем тип новый, а потом просим прописать функцию для сравнения
 * по сложному реализовать парсер
 */

static InputMode ReadModeFromUser() {
    std::cout << "Choose type:\n"
                 "1 - INT\n"
                 "2 - STRING\n"
                 "3 - SET_INT\n"
                 "> ";
    int m = 0;
    if (!(std::cin >> m)) throw std::runtime_error("Bad type input");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (m) {
        case 1: return InputMode::INT;
        case 2: return InputMode::STRING;
        case 3: return InputMode::SET_INT;
        default: throw std::runtime_error("Type must be 1/2/3");
    }
}
static int ReadInputSourceFromUser() {
    std::cout << "Input source:\n"
                 "1 - Console\n"
                 "2 - File\n"
                 "> ";
    int src = 0;
    if (!(std::cin >> src)) throw std::runtime_error("Bad source input");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (src != 1 && src != 2) throw std::runtime_error("Source must be 1 or 2");
    return src;
}
static std::vector<Element> ReadElements(InputMode mode, int src) {
    if (src == 1) {
        std::cout << "Enter elements, one per line. Empty line finishes.\n";
        return ReadElementsFromLines(std::cin, mode);
    }

    // src == 2
    std::cout << "Enter file path: ";
    std::string path;
    std::getline(std::cin, path);

    std::ifstream fin(path);
    if (!fin) throw std::runtime_error("Cannot open file: " + path);

    return ReadElementsFromLines(fin, mode);
}
static std::vector<Element> DeduplicateStable(const std::vector<Element>& in, int& removed) {
    removed = 0;
    std::vector<Element> out;
    out.reserve(in.size());

    for (const auto& e : in) {
        bool seen = false;
        for (const auto& x : out) {
            if (x == e) {
                seen = true;
                break;
            }
        }
        if (seen) ++removed;
        else out.push_back(e);
    }
    return out;
}
static Rules ReadRuleFromUser(Element::Type mode) {
    if (mode == Element::Type::INT) {
        std::cout << "Choose rule for INT:\n"
                     "1 - a divides b (a|b)\n"
                     "2 - a <= b\n"
                     "> ";
        int r = 0;
        if (!(std::cin >> r)) throw std::runtime_error("Bad rule input");
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (r == 1) return Rules::ForInt(Rules::IntRule::DIVIDES);
        if (r == 2) return Rules::ForInt(Rules::IntRule::LEQ);
        throw std::runtime_error("Rule must be 1 or 2");
    }

    if (mode == Element::Type::STRING) {
        std::cout << "Choose rule for STRING:\n"
                     "1 - prefix order (x is prefix of y)\n"
                     "2 - lexicographic order\n"
                     "3 - subsequence order\n"
                     "> ";
        int r = 0;
        if (!(std::cin >> r)) throw std::runtime_error("Bad rule input");
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (r == 1) return Rules::ForString(Rules::StringRule::PREFIX);
        if (r == 2) return Rules::ForString(Rules::StringRule::LEX);
        if (r == 3) return Rules::ForString(Rules::StringRule::SUBSEQ);
        throw std::runtime_error("Rule must be 1 or 2 or 3");
    }

    if (mode == Element::Type::SET_INT) {
        std::cout << "Choose rule for SET_INT:\n"
                     "1 - subset order (A ⊆ B)\n"
                     "2 - by size (|A| <= |B|)\n"
                     "> ";
        int r = 0;
        if (!(std::cin >> r)) throw std::runtime_error("Bad rule input");
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (r == 1) return Rules::ForSet(Rules::SetRule::SUBSET);
        if (r == 2) return Rules::ForSet(Rules::SetRule::SIZE);
        throw std::runtime_error("Rule must be 1 or 2");
    }

    throw std::runtime_error("Unsupported element type");
}
static void PrintElements(const std::vector<Element>& elements) {
    std::cout << "Elements (" << elements.size() << "):\n";
    for (size_t i = 0; i < elements.size(); ++i) {
        std::cout << "  [" << i << "] " << elements[i].ToString() << "\n";
    }
}

int main() {
    try {
        std::cout << "Choose what you want to do:\n1 - Check base HasseDiagram\n2 - See a real-world application of the HasseDiagram in Bioinformatics\n> ";
        int res;
        std::cin >> res;
        if (res == 1) {
            InputMode mode = ReadModeFromUser();
            int src = ReadInputSourceFromUser();
            std::vector<Element> elements = ReadElements(mode, src);
            const Element::Type expected = ModeToElementType(mode);
            for (const auto& e : elements) {
                if (e.GetType() != expected) {
                    throw std::runtime_error("Internal error: mixed element types");
                }
            }
            int removed = 0;
            elements = DeduplicateStable(elements, removed);
            if (removed > 0) {
                std::cout << "Removed duplicates: " << removed << "\n";
            }
            PrintElements(elements);
            std::cout << "Choose how you will make pairs:\n1 - By rule\n2 - Just by pairs\n";
            int res1;
            std::cin >> res1;
            if (res1 == 1) {
                Rules rules = ReadRuleFromUser(expected);
                const auto edges = HasseBuilder::BuildHasseEdges(elements, rules);

                std::cout << "\nHasse edges (" << edges.size() << "):\n";
                for (const auto& [u, v] : edges) {
                    std::cout << elements[u].ToString() << " -> " << elements[v].ToString() << "\n";
                }
                std::ofstream("hasse.dot") << HasseBuilder::ToDot(elements, edges);
                std::cout << "Saved hasse.dot\n";
                std::vector<DrawVertex> vertices = VerticesFromHasse(elements, edges);
                std::cout << "HasseDiagram was saved in screenshot.png\n";
                DrawHasse(vertices, edges);
            } else {
                std::cout << "Number of pairs: ";
                int number;
                std::cin >> number;
                std::vector<HasseBuilder::Edge> edges;
                std::cout << "Write every pair in line:\n";
                for (int i = 0; i < number; i++) {
                    Element x1, x2;
                    if (mode == InputMode::INT) {
                        int a1, a2;
                        std::cin >> a1 >> a2;
                        auto find1 = std::find(elements.begin(), elements.end(), a1);
                        auto find2 = std::find(elements.begin(), elements.end(), a2);
                        if (find1 != elements.end() && find2 != elements.end()) {
                            int index1 = static_cast<int>(std::distance(elements.begin(), find1));
                            int index2 = static_cast<int>(std::distance(elements.begin(), find2));
                            edges.emplace_back(index1, index2);
                        } else {
                            throw std::runtime_error("This element wasn't before");
                        }
                    } else if (mode == InputMode::STRING) {
                        std::string a1, a2;
                        std::cin >> a1 >> a2;
                        auto find1 = std::find(elements.begin(), elements.end(), a1);
                        auto find2 = std::find(elements.begin(), elements.end(), a2);
                        if (find1 != elements.end() && find2 != elements.end()) {
                            int index1 = static_cast<int>(std::distance(elements.begin(), find1));
                            int index2 = static_cast<int>(std::distance(elements.begin(), find2));
                            edges.emplace_back(index1, index2);
                        } else {
                            throw std::runtime_error("This element wasn't before");
                        }
                    } else if (mode == InputMode::SET_INT) {
                        std::string block;
                        std::vector<std::vector<int>> result;
                        while (std::getline(std::cin, block, ';')) {
                            std::stringstream nums(block);
                            std::vector<int> v;
                            int x;

                            while (nums >> x) {
                                v.push_back(x);
                            }
                            if (!v.empty())
                                result.push_back(v);
                        }
                        std::vector<int> a1 = result[0];
                        std::vector<int> a2 = result[1];
                        auto find1 = std::find(elements.begin(), elements.end(), a1);
                        auto find2 = std::find(elements.begin(), elements.end(), a2);
                        if (find1 != elements.end() && find2 != elements.end()) {
                            int index1 = static_cast<int>(std::distance(elements.begin(), find1));
                            int index2 = static_cast<int>(std::distance(elements.begin(), find2));
                            edges.emplace_back(index1, index2);
                        } else {
                            throw std::runtime_error("This element wasn't before");
                        }
                    }
                }
                std::cout << "\nHasse edges (" << edges.size() << "):\n";
                for (const auto& [u, v] : edges) {
                    std::cout << elements[u].ToString() << " -> " << elements[v].ToString() << "\n";
                }
                std::ofstream("hasse.dot") << HasseBuilder::ToDot(elements, edges);
                std::cout << "Saved hasse.dot\n";
                std::vector<DrawVertex> vertices = VerticesFromHasse(elements, edges);
                std::cout << "HasseDiagram was saved in screenshot.png\n";
                DrawHasse(vertices, edges);
            }
            return 0;
        } else {
            readCSV();
            InputMode mode = InputMode::STRING;
            int src = ReadInputSourceFromUser();
            std::vector<Element> elements = ReadElements(mode, src);
            const Element::Type expected = ModeToElementType(mode);
            for (const auto& e : elements) {
                if (e.GetType() != expected) {
                    throw std::runtime_error("Internal error: mixed element types");
                }
            }
            int removed = 0;
            elements = DeduplicateStable(elements, removed);
            if (removed > 0) {
                std::cout << "Removed duplicates: " << removed << "\n";
            }
            std::vector<Element> RealElements;
            for (const auto & element : elements) {
                if (CheckSeq(element.AsString())) {
                    RealElements.push_back(element);
                } else {
                    throw std::runtime_error(std::format("String data '{}' is not a sequence of aminoacids", element.AsString()));
                }
            }
            PrintElements(RealElements);
            Rules rules = Rules::ForString(Rules::StringRule::SUBSEQ);
            const auto edges = HasseBuilder::BuildHasseEdges(elements, rules);
            std::cout << "\nHasse edges (" << edges.size() << "):\n";
            for (const auto& [u, v] : edges) {
                std::cout << elements[u].ToString() << " -> " << elements[v].ToString() << "\n";
            }
            std::ofstream("hasse.dot") << HasseBuilder::ToDot(elements, edges);
            std::cout << "Saved hasse.dot\n";
            std::vector<DrawVertex> vertices = VerticesFromHasse(elements, edges);
            std::cout << "HasseDiagram was saved in screenshot.png\n";
            DrawHasseBio(vertices, edges);
            return 0;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}