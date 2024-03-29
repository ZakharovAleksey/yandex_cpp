#include "cell.h"

#include <optional>
#include <string>

/* CELL VALUE INTERFACE */

class Cell::CellValueInterface {
public:  // Types
    using Value = CellInterface::Value;

    enum class Type { Empty, Text, Formula };

public:  // Constructor
    explicit CellValueInterface(Type type) : type_(type) {}

public:  // Destructor
    virtual ~CellValueInterface() = default;

public:  // Methods
    [[nodiscard]] virtual Value GetValue() const = 0;
    [[nodiscard]] virtual Value GetRawValue() const = 0;
    [[nodiscard]] virtual std::string GetText() const = 0;

    [[nodiscard]] Type GetType() const {
        return type_;
    }

private:  // Fields
    Type type_;
};

/* EMPTY CELL VALUE */

class Cell::EmptyCellValue : public CellValueInterface {
public:  // Constructor
    EmptyCellValue() : CellValueInterface(CellValueInterface::Type::Empty) {}

public:  // Methods
    [[nodiscard]] Value GetValue() const override {
        return 0.;
    }
    [[nodiscard]] Value GetRawValue() const override {
        return 0.;
    }
    [[nodiscard]] std::string GetText() const override {
        using namespace std::string_literals;
        return ""s;
    }
};

/* TEXT CELL VALUE */

class Cell::TextCellValue : public CellValueInterface {
public:  // Constructor
    explicit TextCellValue(std::string text)
        : CellValueInterface(CellValueInterface::Type::Text), text_(std::move(text)) {}

public:  // Methods
    [[nodiscard]] Value GetValue() const override {
        return (!text_.empty() && text_.front() == kEscapeSign) ? text_.substr(1) : text_;
    }
    [[nodiscard]] Value GetRawValue() const override {
        return text_;
    }
    [[nodiscard]] std::string GetText() const override {
        return text_;
    }

private:  // Fields
    std::string text_;
};

/* FORMULA CELL VALUE */

class Cell::FormulaCellValue : public CellValueInterface {
public:  // Types
    struct FormulaEvaluationGetter {
        CellInterface::Value operator()(double value) {
            return value;
        }

        Cell::CellValueInterface::Value operator()(FormulaError error) {
            return error;
        }
    };

public:  // Constructor
    FormulaCellValue(std::string text, SheetInterface& sheet)
        : CellValueInterface(CellValueInterface::Type::Formula),
          formula_(ParseFormula(std::move(text))),
          sheet_(sheet) {}

public:  // Methods
    Value GetValue() const override {
        if (cache_.has_value())
            return cache_.value();

        auto evaluation_result = formula_->Evaluate(sheet_);
        if (auto* value = std::get_if<double>(&evaluation_result))
            cache_ = *value;

        return std::visit(FormulaEvaluationGetter{}, evaluation_result);
    }

    Value GetRawValue() const override {
        return GetValue();
    }
    std::string GetText() const override {
        using namespace std::string_literals;
        return "="s + formula_->GetExpression();
    }
    std::vector<Position> GetReferencedCells() const {
        return formula_->GetReferencedCells();
    }

    bool IsCacheValid() const {
        return cache_.has_value();
    }
    void InvalidateCache() {
        cache_.reset();
    }

private:  // Fields
    std::unique_ptr<FormulaInterface> formula_{nullptr};
    SheetInterface& sheet_;
    mutable std::optional<double> cache_;
};

/* MAIN CELL CLASS */

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
        if (!descending_cells_.empty())
            RemoveOldConnections();

        if (!ascending_cells_.empty()) {
            CellsStorage visited{this};
            InvalidateReferencedCellsCache(visited);
        }

        value_.reset();
    }
    value_ = std::make_unique<EmptyCellValue>();
}

Cell::Value Cell::GetValue() const {
    return value_->GetValue();
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

Cell::CellValueIntefaceUPtr Cell::TryCreateCell(std::string text, SheetInterface& sheet) const {
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

Cell::FormulaCellValue* Cell::TryInterpretAsFormula(const CellValueIntefaceUPtr& cell) const {
    return (cell && cell->GetType() == CellValueInterface::Type::Formula) ? dynamic_cast<FormulaCellValue*>(cell.get())
                                                                          : nullptr;
}

bool Cell::IsReferenced() const {
    if (auto* formula_cell = TryInterpretAsFormula(value_))
        return !formula_cell->GetReferencedCells().empty();

    return false;
}

void Cell::GetReferencedCellsImpl(std::vector<Position>& referenced, CellsStorage& visited) const {
    auto* formula_cell = TryInterpretAsFormula(value_);

    if (!formula_cell)
        return;

    for (const auto& position : formula_cell->GetReferencedCells()) {
        if (auto* cell = GetCell(position); visited.count(cell) == 0) {
            referenced.emplace_back(position);
            visited.insert(cell);

            cell->GetReferencedCellsImpl(referenced, visited);
        }
    }
}

bool Cell::HasCircularDependency(const Cell* reference, const CellValueIntefaceUPtr& current,
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

void Cell::InstantiateCellsIfNotExists(const CellValueIntefaceUPtr& current) {
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
        cell->ascending_cells_.erase(this);

    descending_cells_.clear();
}

void Cell::EstablishNewConnections(FormulaCellValue* formula_cell) {
    if (!formula_cell)
        return;

    for (auto& position : formula_cell->GetReferencedCells()) {
        auto* cell = GetCell(position);

        descending_cells_.insert(cell);
        cell->ascending_cells_.insert(this);
    }
}
