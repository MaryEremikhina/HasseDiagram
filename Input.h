#ifndef AUTOLABA_INPUT_H
#define AUTOLABA_INPUT_H

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

enum class InputMode { INT, STRING, SET_INT };

static std::string trim(const std::string& s) {
    size_t l = 0;
    while (l < s.size() && std::isspace((unsigned char)s[l])) ++l;
    size_t r = s.size();
    while (r > l && std::isspace((unsigned char)s[r - 1])) --r;
    return s.substr(l, r - l);
}

static Element ParseElementFromLine(InputMode mode, const std::string& rawLine) {
    std::string line = trim(rawLine);
    if (line.empty()) throw std::runtime_error("Empty element line");

    if (mode == InputMode::INT) {
        try {
            int x = std::stoi(line);
            return Element(x);
        } catch (...) {
            throw std::runtime_error("Bad INT line: '" + line + "'");
        }
    }

    if (mode == InputMode::STRING) {
        return Element(line);
    }

    std::istringstream iss(line);
    std::vector<int> v;
    int x;
    while (iss >> x) v.push_back(x);

    if (v.empty()) {
        throw std::runtime_error("Bad SET_INT line (no numbers): '" + line + "'");
    }
    return Element(std::move(v));
}

static std::vector<Element> ReadElementsFromLines(std::istream& in, InputMode mode) {
    std::vector<Element> elements;
    std::string line;

    while (true) {
        if (!std::getline(in, line)) break;
        if (trim(line).empty()) break;

        elements.push_back(ParseElementFromLine(mode, line));
    }

    if (elements.empty()) throw std::runtime_error("No elements were provided");
    return elements;
}

static Element::Type ModeToElementType(InputMode mode) {
    if (mode == InputMode::INT) return Element::Type::INT;
    if (mode == InputMode::STRING) return Element::Type::STRING;
    return Element::Type::SET_INT;
}

#endif //AUTOLABA_INPUT_H