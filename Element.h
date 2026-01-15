#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>


#ifndef AUTOLABA_ELEMENT_H
#define AUTOLABA_ELEMENT_H

class Element {
private:
    std::string str;
    int val = 0;
    std::vector<int> set_int;

    public:

    enum class Type { NONE, STRING, INT, SET_INT };
    Type type = Type::NONE;

    Element() = default;
    Element(std::string str) : str(std::move(str)), type(Type::STRING) {}
    Element(int val) : val(val), type(Type::INT) {}
    Element(std::vector<int> v) {
        type = Type::SET_INT;
        std::vector<int> set = std::move(v);
        std::sort(set.begin(), set.end());

        std::vector<int> set1;
        if (!set.empty()) {
            set1.push_back(set[0]);
            for (size_t i = 1; i < set.size(); i++) {
                if (set[i] != set[i - 1]) {
                    set1.push_back(set[i]);
                }
            }
        }
        set_int = std::move(set1);
    }

    ~Element() = default;

    Type GetType() const { return type; }

    bool IsString() const { return type == Type::STRING; }
    bool IsInt() const { return type == Type::INT; }
    bool IsSetInt() const { return type == Type::SET_INT; }
    bool IsNone() const { return type == Type::NONE; }

    const std::string& AsString() const {
        if (type == Type::STRING) {
            return str;
        }
        else {
            throw std::runtime_error("Element is not a string");
        }
    }

    int AsInt() const {
        if (type == Type::INT) {
            return val;
        }
        else {
            throw std::runtime_error("Ellement is not an integer");
        }
    }

    const std::vector<int>& AsSetInt() const {
        if (type == Type::SET_INT) {
            return set_int;
        }
        else {
            throw std::runtime_error("Ellement is not set of numbers");
        }
    }

    std::string ToString() const {
        switch (type) {
            case Type::STRING: return str;
            case Type::INT:    return std::to_string(val);
            case Type::SET_INT: {
                std::string s;
                s.push_back('[');
                for (size_t i = 0; i < set_int.size(); i++) {
                    s += std::to_string(set_int.at(i));
                    if (i + 1 != set_int.size()) s += ", ";
                }
                s.push_back(']');
                return s;
            }
            case Type::NONE: return "NONE";
        }
        return "NONE";
    }

    bool operator==(const Element& other) const {
        if (type != other.type) return false;
            switch (type) {
                case Type::STRING: return str == other.str;
                case Type::INT:    return val == other.val;
                case Type::SET_INT: return set_int == other.set_int;
                case Type::NONE: return true;
            }
            return false;
    }
};
#endif //AUTOLABA_ELEMENT_H