#include "cell.h"

#include <optional>
#include <string>

namespace {
std::unique_ptr<CellValueInterface> TryCreateCell(std::string text, SheetInterface& sheet) {
    using namespace std::string_literals;

    try {
        if (text.size() > 1 && text.front() == kFormulaSign) {
            return std::make_unique<FormulaCellValue>(text.substr(1), sheet);
        } else if (!text.empty()) {
            return std::make_unique<TextCellValue>(std::move(text));
        } else {
            return std::make_unique<EmptyCellValue>();
        }
    } catch (std::exception&) {
        throw FormulaException("Error in Set() method call for text: "s + text);
    }
}

FormulaCellValue* TryInterpretAsFormula(const std::unique_ptr<CellValueInterface>& cell) {
    return cell->GetType() == CellValueInterface::Type::Formula ? dynamic_cast<FormulaCellValue*>(cell.get()) : nullptr;
}

}  // namespace

/* CELL VALUE */

CellValueInterface::CellValueInterface(CellValueInterface::Type type) : type_(type) {}

CellValueInterface::Type CellValueInterface::GetType() const {
    return type_;
}

/* EMPTY CELL VALUE */

EmptyCellValue::EmptyCellValue() : CellValueInterface(CellValueInterface::Type::Empty) {}

CellValueInterface::Value EmptyCellValue::GetValue() const {
    return 0.;
}
CellValueInterface::Value EmptyCellValue::GetRawValue() const {
    return 0.;
}
std::string EmptyCellValue::GetText() const {
    using namespace std::string_literals;
    return ""s;
}

/* TEXT CELL VALUE */

TextCellValue::TextCellValue(std::string text)
    : CellValueInterface(CellValueInterface::Type::Text), text_(std::move(text)) {}

CellValueInterface::Value TextCellValue::GetValue() const {
    return (!text_.empty() && text_.front() == kEscapeSign) ? text_.substr(1) : text_;
}
CellValueInterface::Value TextCellValue::GetRawValue() const {
    return text_;
}
std::string TextCellValue::GetText() const {
    return text_;
}

/* FORMULA CELL VALUE */

struct FormulaEvaluationGetter {
    CellInterface::Value operator()(double value) {
        return value;
    }

    CellValueInterface::Value operator()(FormulaError error) {
        return error;
    }
};

FormulaCellValue::FormulaCellValue(std::string text, SheetInterface& sheet)
    : CellValueInterface(CellValueInterface::Type::Formula), formula_(ParseFormula(std::move(text))), sheet_(sheet) {}

CellValueInterface::Value FormulaCellValue::GetValue() const {
    if (!cache_)
        cache_ = std::visit(FormulaEvaluationGetter{}, formula_->Evaluate(sheet_));

    return cache_.value();
}
CellValueInterface::Value FormulaCellValue::GetRawValue() const {
    return GetValue();
}
std::string FormulaCellValue::GetText() const {
    using namespace std::string_literals;
    return "="s + formula_->GetExpression();
}

std::vector<Position> FormulaCellValue::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

bool FormulaCellValue::IsCacheValid() const {
    return cache_.has_value();
}

void FormulaCellValue::InvalidateCache() {
    cache_.reset();
}

// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : sheet_(sheet) {}

void Cell::Set(std::string text) {
    // Step 1. Create temporary cell interface
    // NOTE: Temporary because cell should not exist till circular dependency check is ok
    auto tmp = TryCreateCell(text, sheet_);

    // Step 2. Instantiate cell for formula in case they do not exist but references in formula
    InstantiateCellsIfNotExists(tmp);

    // Step 3. Check on circular cell dependency
    if (CellsStorage visited{this}; HasCircularDependency(this, tmp, visited))
        throw CircularDependencyException("");

    // Step 4. Make temporary value primary
    value_ = std::move(tmp);

    // Step 5. If previously this cell was a formula - remove cells
    if (!descending_cells_.empty())
        RemoveOldConnections();

    // Step 6. If now this cell contains a formula - create connections
    if (auto* formula_cell = TryInterpretAsFormula(value_))
        EstablishNewConnections(formula_cell);

    // Step 7. If there are any dependencies from this cell via formula - invalidate them
    if (!ascending_cells_.empty()) {
        CellsStorage visited{this};
        InvalidateReferencedCellsCache(visited);
    }
}

void Cell::Clear() {
    if (value_) {
        CellsStorage visited{this};
        InvalidateReferencedCellsCache(visited);

        // In case, cell is formula - remove all references to this cell on the referenced
        if (value_->GetType() == CellValueInterface::Type::Formula) {
            for (const auto& position : TryInterpretAsFormula(value_)->GetReferencedCells()) {
                const auto* cell = dynamic_cast<Cell*>(sheet_.GetCell(position));
                cell->RemoveReference(this);
            }
        }

        value_.reset();
    }
    value_ = std::make_unique<EmptyCellValue>();
}

Cell::Value Cell::GetValue() const {
    return value_->GetValue();
}

Cell::Value Cell::GetRawValue() const {
    return value_->GetRawValue();
}

std::string Cell::GetText() const {
    return value_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    if (!value_ || value_->GetType() != CellValueInterface::Type::Formula)
        return {};

    std::vector<Position> referenced_cells;
    CellsStorage visited;
    GetReferencedCellsImpl(referenced_cells, visited);

    return referenced_cells;
}

bool Cell::IsReferenced() const {
    if (auto* formula_cell = TryInterpretAsFormula(value_))
        return !formula_cell->GetReferencedCells().empty();

    return false;
}

void Cell::AddReference(const Cell* cell) const {
    ascending_cells_.insert(cell);
}

void Cell::RemoveReference(const Cell* cell) const {
    ascending_cells_.erase(cell);
}

void Cell::GetReferencedCellsImpl(std::vector<Position>& referenced, CellsStorage& visited) const {
    if (!value_ || value_->GetType() != CellValueInterface::Type::Formula)
        return;

    for (const auto& position : TryInterpretAsFormula(value_)->GetReferencedCells()) {
        if (auto* cell = GetCell(position); visited.count(cell) == 0) {
            referenced.emplace_back(position);
            visited.insert(cell);

            cell->GetReferencedCellsImpl(referenced, visited);
        }
    }
}

bool Cell::HasCircularDependency(const Cell* reference, const std::unique_ptr<CellValueInterface>& current,
                                 CellsStorage& visited) const {
    auto* formula_cell = TryInterpretAsFormula(current);

    // In case this is not a formula - no need to check further because there is no references on other cells
    if (!formula_cell)
        return false;

    for (const auto& position : formula_cell->GetReferencedCells()) {
        auto* cell = GetCell(position);

        if (cell == reference)
            return true;

        if (visited.count(cell) == 0) {
            visited.insert(cell);

            if (cell->HasCircularDependency(reference, cell->value_, visited))
                return true;
        }
    }

    return false;
}

void Cell::InvalidateReferencedCellsCache(CellsStorage& visited) const {
    for (const auto* cell : ascending_cells_) {
        if (visited.count(cell) > 0)
            return;

        visited.insert(cell);

        // Do invalidation only in case cache is valid (otherwise - no need to do check)
        if (auto* formula_cell = TryInterpretAsFormula(cell->value_); formula_cell && formula_cell->IsCacheValid()) {
            formula_cell->InvalidateCache();
            cell->InvalidateReferencedCellsCache(visited);
        }
    }
}

const Cell* Cell::GetCell(Position position) const {
    return dynamic_cast<const Cell*>(sheet_.GetCell(position));
}

void Cell::InstantiateCellsIfNotExists(const std::unique_ptr<CellValueInterface>& current) {
    using namespace std::string_literals;

    if (auto* formula_cell = TryInterpretAsFormula(current)) {
        for (const auto& position : formula_cell->GetReferencedCells()) {
            if (auto* cell = GetCell(position); !cell)
                sheet_.SetCell(position, ""s);
        }
    }
}

void Cell::RemoveOldConnections() {
    // Remove connection between ascending cells and this
    for (auto* cell : descending_cells_)
        cell->RemoveAscendingCell(this);

    descending_cells_.clear();
}

void Cell::EstablishNewConnections(FormulaCellValue* formula_cell) {
    if (!formula_cell)
        return;

    for (auto& position : formula_cell->GetReferencedCells()) {
        auto* cell = GetCell(position);

        descending_cells_.insert(cell);
        cell->AddAscendingCell(this);
    }
}
