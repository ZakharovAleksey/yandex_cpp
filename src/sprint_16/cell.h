#pragma once

#include <optional>
#include <unordered_set>

#include "common.h"
#include "formula.h"

class CellValueInterface {
public:  // Types
    using Value = CellInterface::Value;

    enum class Type { Empty, Text, Formula };

public:  // Constructor
    explicit CellValueInterface(Type type);

public:  // Destructor
    virtual ~CellValueInterface() = default;

public:  // Methods
    virtual Value GetValue() const = 0;
    virtual Value GetRawValue() const = 0;
    virtual std::string GetText() const = 0;
    [[nodiscard]] Type GetType() const;

private:  // Fields
    Type type_;
};

class EmptyCellValue : public CellValueInterface {
public:  // Constructor
    EmptyCellValue();

public:  // Methods
    Value GetValue() const override;
    Value GetRawValue() const override;
    std::string GetText() const override;
};

class TextCellValue : public CellValueInterface {
public:  // Constructor
    TextCellValue(std::string text);

public:  // Methods
    Value GetValue() const override;
    Value GetRawValue() const override;
    std::string GetText() const override;

private:  // Fields
    std::string text_;
};

class FormulaCellValue : public CellValueInterface {
public:  // Constructor
    FormulaCellValue(std::string text, SheetInterface& sheet);

public:  // Methods
    Value GetValue() const override;
    Value GetRawValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const;
    void ClearCache();

private:  // Fields
    std::unique_ptr<FormulaInterface> formula_{nullptr};
    SheetInterface& sheet_;
    mutable std::optional<Value> cache_;
};

class Cell : public CellInterface {
public:  // Constructor
    Cell(SheetInterface& sheet);

public:  // Destructor
    virtual ~Cell() = default;

public:  // Methods
    void Set(std::string text) override;
    void Clear();

    Value GetValue() const override;
    Value GetRawValue() const;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    void AddReference(const Cell* cell) const;
    void RemoveReference(const Cell* cell) const;

private:
    void GetReferencedCellsImpl(std::vector<Position>& referenced, std::unordered_set<const Cell*>& visited) const;
    bool HasCircularDependency(const Cell* reference, const std::unique_ptr<CellValueInterface>& current,
                               std::unordered_set<const Cell*>& visited) const;
    void InvalidateReferencedCellsCache(std::unordered_set<const Cell*>& visited) const;

private:
    SheetInterface& sheet_;
    std::unique_ptr<CellValueInterface> value_{nullptr};
    mutable std::unordered_set<const Cell*> referenced_cells_;
};