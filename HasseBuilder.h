#ifndef AUTOLABA_HASSEBUILDER_H
#define AUTOLABA_HASSEBUILDER_H

#include <vector>
#include <utility>
#include <string>
#include <sstream>

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

        out << "}\n";
        return out.str();
    }
};

#endif //AUTOLABA_HASSEBUILDER_H