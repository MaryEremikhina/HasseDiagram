#ifndef AUTOLABA_RULES_H
#define AUTOLABA_RULES_H

#include <string>
#include <vector>
#include <stdexcept>

class Rules {
public:
    enum class Cmp { Less, Greater, Equal, Incomparable};
    enum class StringRule { PREFIX, LEX, SUBSEQ };
    enum class IntRule { DIVIDES, LEQ };
    enum class SetRule { SUBSET, SIZE };

    explicit Rules(Element::Type mode) : mode(mode) {
        if (mode == Element::Type::NONE) {
            throw std::runtime_error("Rules: mode cannot be NONE");
        }
    }

    Element::Type GetMode() const {return mode;}

    Cmp Compare(const Element& a, const Element& b) const {
        if (!rule_selected_) {
            throw std::runtime_error("Rules: no rule selected (choose a rule explicitly)");
        }
        if (a.GetType() != mode || b.GetType() != mode) {
            throw std::runtime_error("Rules: element type does not match mode");
        }

        switch (mode) {
            case Element::Type::STRING:
                if (string_rule_ == StringRule::PREFIX) {
                    return CompareStringsPrefix(a.AsString(), b.AsString());
                } else if (string_rule_ == StringRule::LEX) {
                    return CompareStringsLex(a.AsString(), b.AsString());
                } else {
                    return CompareStringsSubSeq(a.AsString(), b.AsString());
                }

            case Element::Type::INT:
                return (int_rule_ == IntRule::DIVIDES)
                    ? CompareIntsDivides(a.AsInt(), b.AsInt())
                    : CompareIntsLeq(a.AsInt(), b.AsInt());

            case Element::Type::SET_INT:
                return (set_rule_ == SetRule::SUBSET)
                    ? CompareSetsSubset(a.AsSetInt(), b.AsSetInt())
                    : CompareSetsSize(a.AsSetInt(), b.AsSetInt());

            case Element::Type::NONE: return Cmp::Incomparable;
        }
        return Cmp::Incomparable;
    }

    static Rules ForString(StringRule r) {
        Rules rules(Element::Type::STRING);
        rules.string_rule_ = r;
        rules.rule_selected_ = true;
        return rules;
    }

    static Rules ForInt(IntRule r) {
        Rules rules(Element::Type::INT);
        rules.int_rule_ = r;
        rules.rule_selected_ = true;
        return rules;
    }

    static Rules ForSet(SetRule r) {
        Rules rules(Element::Type::SET_INT);
        rules.set_rule_ = r;
        rules.rule_selected_ = true;
        return rules;
    }

private:
    Element::Type mode;
    StringRule string_rule_ = StringRule::PREFIX;
    IntRule int_rule_ = IntRule::DIVIDES;
    SetRule set_rule_ = SetRule::SUBSET;
    bool rule_selected_ = false;

    static Cmp CompareStringsPrefix(const std::string& x, const std::string& y) {
        if (x == y) return Cmp::Equal;
        if (IsPrefix(x, y)) return Cmp::Less;
        if (IsPrefix(y, x)) return Cmp::Greater;
        return Cmp::Incomparable;
    }

    static Cmp CompareStringsLex(const std::string& x, const std::string& y) {
        if (x == y) return Cmp::Equal;
        return (x < y) ? Cmp::Less : Cmp::Greater;
    }

    static Cmp CompareStringsSubSeq(const std::string& x, const std::string& y) {
        if (x == y) return Cmp::Equal;
        if (IsPart(x, y)) return Cmp::Less;
        if (IsPart(y, x)) return Cmp::Greater;
        return Cmp::Incomparable;
    }

    static bool IsPart(const std::string& seq1, const std::string& seq2) {
        int index1 = 0;
        int index2 = 0;
        while (index1 < seq1.size() && index2 < seq2.size()) {
            if (seq1[index1] == seq2[index2]) {
                index1++;
                index2++;
            } else {
                index2++;
            }
        }
        if (index1 == seq1.size()) {
            return true;
        }
        return false;
    }

    static bool IsPrefix(const std::string& pref, const std::string& s) {
        if (pref.size() > s.size()) return false;
        return s.rfind(pref, 0) == 0;
    }

    static Cmp CompareIntsDivides(int a, int b) {
        if (a == b) return Cmp::Equal;
        const bool a_div_b = Divides(a, b);
        const bool b_div_a = Divides(b, a);
        if (a_div_b) return Cmp::Less;
        if (b_div_a) return Cmp::Greater;
        return Cmp::Incomparable;
    }

    static Cmp CompareIntsLeq(int a, int b) {
        if (a == b) return Cmp::Equal;
        return (a < b) ? Cmp::Less : Cmp::Greater;
    }

    static bool Divides(int a, int b) {
        if (a == 0) return b == 0;
        return (b % a) == 0;
    }

    static Cmp CompareSetsSubset(const std::vector<int>& A, const std::vector<int>& B) {
        if (A == B) return Cmp::Equal;
        const bool A_in_B = IsSubset(A, B);
        const bool B_in_A = IsSubset(B, A);
        if (A_in_B) return Cmp::Less;
        if (B_in_A) return Cmp::Greater;
        return Cmp::Incomparable;
    }

    static Cmp CompareSetsSize(const std::vector<int>& A, const std::vector<int>& B) {
        if (A == B) return Cmp::Equal;
        if (A.size() < B.size()) return Cmp::Less;
        if (A.size() > B.size()) return Cmp::Greater;
        return Cmp::Incomparable;
    }

    static bool IsSubset(const std::vector<int>& A, const std::vector<int>& B) {
        size_t i = 0, j = 0;
        while (i < A.size() && j < B.size()) {
            if (A[i] == B[j]) { ++i; ++j; }
            else if (A[i] > B[j]) { ++j; }
            else return false;
        }
        return i == A.size();
    }
};
#endif //AUTOLABA_RULES_H
