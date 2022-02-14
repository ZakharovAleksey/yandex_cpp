#include "cell.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <string>

namespace {
std::unique_ptr<CellValueInterface> TryCreateCell(std::string text, SheetInterface& sheet) {
    using namespace std::string_literals;

    try {
        if (text.size() > 1 && text.front() == FORMULA_SIGN) {
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
    return (!text_.empty() && text_.front() == ESCAPE_SIGN) ? text_.substr(1) : text_;
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

void FormulaCellValue::ClearCache() {
    cache_.reset();
}

// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : sheet_(sheet) {}

void Cell::Set(std::string text) {
    auto tmp = TryCreateCell(text, sheet_);
    std::unordered_set<const Cell*> visited{this};

    if (HasCircularDependency(this, tmp, visited))
        throw CircularDependencyException("");

    Clear();
    value_ = std::move(tmp);

    // In case, cell is formula - make a reference to this cell for all cells in formula
    if (value_->GetType() != CellValueInterface::Type::Formula)
        return;

    for (const auto& position : dynamic_cast<FormulaCellValue*>(value_.get())->GetReferencedCells()) {
        const auto* cell = dynamic_cast<Cell*>(sheet_.GetCell(position));
        cell->AddReference(cell);
    }
}

void Cell::Clear() {
    if (value_) {
        std::unordered_set<const Cell*> visited{this};
        InvalidateReferencedCellsCache(visited);

        // In case, cell is formula - remove all references to this cell on the referenced
        if (value_->GetType() == CellValueInterface::Type::Formula) {
            for (const auto& position : dynamic_cast<FormulaCellValue*>(value_.get())->GetReferencedCells()) {
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
    std::unordered_set<const Cell*> visited_cells;
    GetReferencedCellsImpl(referenced_cells, visited_cells);

    return referenced_cells;
}

bool Cell::IsReferenced() const {
    if (value_->GetType() == CellValueInterface::Type::Formula)
        return !dynamic_cast<FormulaCellValue*>(value_.get())->GetReferencedCells().empty();

    return false;
}

void Cell::AddReference(const Cell* cell) const {
    referenced_cells_.insert(cell);
}

void Cell::RemoveReference(const Cell* cell) const {
    referenced_cells_.erase(cell);
}

void Cell::GetReferencedCellsImpl(std::vector<Position>& referenced, std::unordered_set<const Cell*>& visited) const {
    if (!value_ || value_->GetType() != CellValueInterface::Type::Formula)
        return;

    auto* formula_cell = dynamic_cast<FormulaCellValue*>(value_.get());
    for (const auto& position : formula_cell->GetReferencedCells()) {
        if (auto* cell = dynamic_cast<const Cell*>(sheet_.GetCell(position)); visited.count(cell) == 0) {
            referenced.emplace_back(position);
            visited.insert(cell);

            cell->GetReferencedCellsImpl(referenced, visited);
        }
    }
}

bool Cell::HasCircularDependency(const Cell* reference, const std::unique_ptr<CellValueInterface>& current,
                                 std::unordered_set<const Cell*>& visited) const {
    if (!current || current->GetType() != CellValueInterface::Type::Formula)
        return false;

    auto* formula_cell = dynamic_cast<FormulaCellValue*>(current.get());
    for (const auto& position : formula_cell->GetReferencedCells()) {
        auto* cell = dynamic_cast<const Cell*>(sheet_.GetCell(position));

        if (cell == reference)
            return true;
        if (!cell) {
            sheet_.SetCell(position, "");
            cell = dynamic_cast<const Cell*>(sheet_.GetCell(position));
        }
        if (visited.count(cell) == 0) {
            visited.insert(cell);
            if (cell->HasCircularDependency(reference, cell->value_, visited))
                return true;
        }
    }

    return false;
}

void Cell::InvalidateReferencedCellsCache(std::unordered_set<const Cell*>& visited) const {
    for (const auto* cell : referenced_cells_) {
        if (visited.count(cell) == 0) {
            visited.insert(cell);

            if (cell->value_->GetType() == CellValueInterface::Type::Formula)
                dynamic_cast<FormulaCellValue*>(cell->value_.get())->ClearCache();

            cell->InvalidateReferencedCellsCache(visited);
        }
    }
}