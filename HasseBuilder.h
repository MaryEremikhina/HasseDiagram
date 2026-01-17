#ifndef AUTOLABA_HASSEBUILDER_H
#define AUTOLABA_HASSEBUILDER_H

#include <vector>
#include <utility>
#include <string>
#include <sstream>
#include <map>

class HasseBuilder {
private:
    static void TransitiveClosure(std::vector<std::vector<char>>& le) {
        const int n = static_cast<int>(le.size());
        for (int k = 0; k < n; ++k) {
            for (int i = 0; i < n; ++i) {
                if (!le[i][k]) continue;
                for (int j = 0; j < n; ++j) {
                    if (le[k][j]) le[i][j] = 1;
                }
            }
        }
    }
    static std::string EscapeDot(const std::string& s) {
        std::string r;
        r.reserve(s.size());
        for (char c : s) {
            if (c == '\\' || c == '\"') r.push_back('\\');
            r.push_back(c);
        }
        return r;
    }

public:
    using Edge = std::pair<int,int>;

    static std::vector<Edge> BuildHasseEdges(const std::vector<Element>& elements, const Rules& rules) {
        const int n = static_cast<int>(elements.size());
        if (n == 0) return {};

        std::vector<std::vector<char>> le(n, std::vector<char>(n, 0));
        for (int i = 0; i < n; ++i) le[i][i] = 1;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;
                const auto cmp = rules.Compare(elements[i], elements[j]);
                if (cmp == Rules::Cmp::Less || cmp == Rules::Cmp::Equal) {
                    le[i][j] = 1;
                } else if (cmp == Rules::Cmp::Greater) {
                    le[j][i] = 1;
                }
            }
        }

        TransitiveClosure(le);

        std::vector<Edge> edges;
        edges.reserve(n * 2);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;

                if (!le[i][j] || le[j][i]) continue;

                bool has_middle = false;
                for (int k = 0; k < n; ++k) {
                    if (k == i || k == j) continue;
                    if (le[i][k] && le[k][j] && !(le[k][i] && le[i][k]) && !(le[j][k] && le[k][j])) {
                        has_middle = true;
                        break;
                    }
                }
                if (!has_middle) edges.emplace_back(i, j);
            }
        }

        return edges;
    }
    // разделение вершин на уровни для удобства красивой визуализации
    static std::map<int, std::vector<int>> levelIndex(const std::vector<Edge> &edges, const std::vector<Element>& elements) {
        const int n = static_cast<int>(elements.size());

        std::vector<std::vector<int>> adj(n);
        std::vector<int> indeg(n, 0);
        for (auto& e : edges) {
            adj[e.first].push_back(e.second);
            indeg[e.second]++;
        }
        std::queue<int> q;
        std::vector<int> level(n, 0);

        for (int i = 0; i < n; ++i)
            if (indeg[i] == 0)
                q.push(i);

        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                level[v] = std::max(level[v], level[u] + 1);
                if (--indeg[v] == 0)
                    q.push(v);
            }
        }
        std::map<int, std::vector<int>> result;
        for (int i = 0; i < n; ++i)
            result[level[i]].push_back(i);
        return result;
    }
    static std::string ToDot(const std::vector<Element>& elements, const std::vector<Edge>& edges) {
        std::ostringstream out;
        out << "digraph Hasse {\n";
        out << "  rankdir=BT;\n";
        out << "  node [shape=circle];\n";

        for (int i = 0; i < (int)elements.size(); ++i) {
            out << "  n" << i << " [label=\"" << EscapeDot(elements[i].ToString()) << "\"];\n";
        }

        for (const auto& [u, v] : edges) {
            out << "  n" << u << " -> n" << v << ";\n";
        }

        // вывод экстремальных характеристик
        std::map<int, std::vector<int>> levels = levelIndex(edges, elements);
        out << "}\n\nExtreme characteristics:\nMinimal elements: [";
        for (int i = 0; i < levels[0].size() - 1; i++) {
            out << elements[levels[0][i]].ToString() << ", ";
        }
        out << elements[levels[0][levels[0].size() - 1]].ToString() << "]\nMaximal elements: [";
        for (int i = 0; i < levels[static_cast<int>(levels.size()) - 1].size() - 1; i++) {
            out << elements[levels[static_cast<int>(levels.size()) - 1][i]].ToString() << ", ";
        }
        out << elements[levels[static_cast<int>(levels.size()) - 1][levels[static_cast<int>(levels.size()) - 1].size() - 1]].ToString() << "]\nHeight: ";
        out << static_cast<int>(levels.size()) << "\nWidth: ";
        int maxWidth = 0;
        for (const auto& [lvl, vec] : levels) {
            if (vec.size() > maxWidth)
                maxWidth = static_cast<int>(vec.size());
        }
        out << maxWidth;
        return out.str();
    }
};

#endif //AUTOLABA_HASSEBUILDER_H