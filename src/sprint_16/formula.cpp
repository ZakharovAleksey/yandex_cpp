#include "formula.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

#include "FormulaAST.h"

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:  // Constructor
    explicit Formula(const std::string& expression) : ast_(ParseFormulaAST(expression)) {}

public:  // Methods
    Value Evaluate() const override {
        try {
            return ast_.Execute();
        } catch (FormulaError& error_code) {
            return error_code;
        }
    }

    std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss);
        return ss.str();
    }

private:  // Fields
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    } catch (FormulaException& error_code) {
        throw error_code;
    }
}