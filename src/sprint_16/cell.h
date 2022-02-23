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
    [[nodiscard]] virtual Value GetValue() const = 0;
    [[nodiscard]] virtual Value GetRawValue() const = 0;
    [[nodiscard]] virtual std::string GetText() const = 0;
    [[nodiscard]] Type GetType() const;

private:  // Fields
    Type type_;
};

class EmptyCellValue : public CellValueInterface {
public:  // Constructor
    EmptyCellValue();

public:  // Methods
    [[nodiscard]] Value GetValue() const override;
    [[nodiscard]] Value GetRawValue() const override;
    [[nodiscard]] std::string GetText() const override;
};

class TextCellValue : public CellValueInterface {
public:  // Constructor
    explicit TextCellValue(std::string text);

public:  // Methods
    [[nodiscard]] Value GetValue() const override;
    [[nodiscard]] Value GetRawValue() const override;
    [[nodiscard]] std::string GetText() const override;

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

    bool IsCacheValid() const;
    void InvalidateCache();

private:  // Fields
    std::unique_ptr<FormulaInterface> formula_{nullptr};
    SheetInterface& sheet_;
    mutable std::optional<Value> cache_;
};

class Cell : public CellInterface {
public:  // Types
    using CellsStorage = std::unordered_set<const Cell*>;

public:  // Constructor
    explicit Cell(SheetInterface& sheet);

public:  // Destructor
    ~Cell() override = default;

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
    void GetReferencedCellsImpl(std::vector<Position>& referenced, CellsStorage& visited) const;
    bool HasCircularDependency(const Cell* reference, const std::unique_ptr<CellValueInterface>& current,
                               CellsStorage& visited) const;
    void InvalidateReferencedCellsCache(CellsStorage& visited) const;

    /* SUPPORT FUNCTIONS */

    const Cell* GetCell(Position position) const;
    void InstantiateCellsIfNotExists(const std::unique_ptr<CellValueInterface>& current);

private:
    SheetInterface& sheet_;
    std::unique_ptr<CellValueInterface> value_{nullptr};

    mutable CellsStorage descending_cells_;
    mutable CellsStorage ascending_cells_;
};