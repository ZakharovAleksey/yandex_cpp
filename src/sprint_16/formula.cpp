#include "formula.h"

#include <algorithm>
#include <sstream>

#include "FormulaAST.h"
#include "cell.h"

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError error) {
    return output << error.ToString();
}

struct CellValueGetter {
    double operator()(const std::string& text) {
        try {
            return std::stod(text);
        } catch (...) {
            throw FormulaError(FormulaError::Category::Value);
        }
    }

    double operator()(double value) {
        return value;
    }

    double operator()(const FormulaError& error) {
        throw error;
    }
};

namespace {
class Formula : public FormulaInterface {
public:  // Constructor
    explicit Formula(const std::string& expression)
        : ast_(ParseFormulaAST(expression)), referenced_cells_(ast_.GetCells().begin(), ast_.GetCells().end()) {
        auto last = std::unique(referenced_cells_.begin(), referenced_cells_.end());

        referenced_cells_.erase(last, referenced_cells_.end());
        std::sort(referenced_cells_.begin(), referenced_cells_.end());
    }

public:  // Methods
    [[nodiscard]] Value Evaluate(const SheetInterface& sheet) const override {
        try {
            auto get_value = [&sheet](const Position& position) {
                if (auto* cell = dynamic_cast<const Cell*>(sheet.GetCell(position)))
                    return std::visit(CellValueGetter{}, cell->GetRawValue());
                // In case the cell is beyond the boundaries or absent - treat as Empty cell
                return 0.;
            };

            return ast_.Execute(get_value);
        } catch (FormulaError& error_code) {
            return error_code;
        }
    }

    [[nodiscard]] std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss);

        return ss.str();
    }

    [[nodiscard]] std::vector<Position> GetReferencedCells() const override {
        return referenced_cells_;
    }

private:  // Fields
    FormulaAST ast_;
    std::vector<Position> referenced_cells_;
};

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(const std::string& expression) {
    try {
        return std::make_unique<Formula>(expression);
    } catch (std::exception& e) {
        throw FormulaException(e.what());
    }
}