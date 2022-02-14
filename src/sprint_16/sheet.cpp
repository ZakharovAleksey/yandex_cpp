#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

#include "cell.h"

using namespace std::literals;

namespace {

void CheckIfPositionValid(Position position) {
    if (!position.IsValid()) {
        std::stringstream ss;
        ss << "Position (" << position.row << "," << position.col << ") is invalid: could not use it in table";
        throw InvalidPositionException(ss.str());
    }
}

}  // namespace

void Sheet::SetCell(Position position, std::string text) {
    CheckIfPositionValid(position);

    if (auto* cell = GetCell(position)) {
        dynamic_cast<Cell*>(cell)->Set(std::move(text));
    } else {
        ResizeStorage(position);
        data_.at(position.row).at(position.col) = std::make_unique<Cell>(*this);
        SetCell(position, std::move(text));
    }
}

const CellInterface* Sheet::GetCell(Position position) const {
    return const_cast<Sheet*>(this)->GetCell(position);
}

CellInterface* Sheet::GetCell(Position position) {
    CheckIfPositionValid(position);

    if (position.row >= static_cast<int>(data_.size()))
        return nullptr;
    if (position.col >= static_cast<int>(data_.at(position.row).size()))
        return nullptr;

    return data_.at(position.row).at(position.col).get();
}

void Sheet::ClearCell(Position position) {
    CheckIfPositionValid(position);

    if (GetCell(position))
        data_.at(position.row).at(position.col).reset();
}

Size Sheet::GetPrintableSize() const {
    int row = 0;
    int col = 0;

    for (int row_id = 0; row_id < static_cast<int>(data_.size()); ++row_id) {
        for (int col_id = 0; col_id < static_cast<int>(data_.at(row_id).size()); ++col_id) {
            if (data_.at(row_id).at(col_id)) {
                col = std::max(col, col_id + 1);
                row = std::max(row, row_id + 1);
            }
        }
    }

    return {row, col};
}

void Sheet::PrintValues(std::ostream& output) const {
    auto print_value = [&output](const CellInterface* ptr_value) {
        std::visit([&](const auto& value) { output << value; }, ptr_value->GetValue());
    };
    Print(output, print_value);
}
void Sheet::PrintTexts(std::ostream& output) const {
    Print(output, [&output](const CellInterface* cell) { output << cell->GetText(); });
}

void Sheet::ResizeStorage(Position position) {
    if (position.row >= static_cast<int>(data_.size()))
        data_.resize(position.row + 1);

    if (position.col >= static_cast<int>(data_.at(position.row).size()))
        data_.at(position.row).resize(position.col + 1);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
