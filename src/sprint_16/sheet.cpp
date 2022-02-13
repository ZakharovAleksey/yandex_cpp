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

std::unique_ptr<CellInterface> CreateNewCell(std::string text) {
    std::unique_ptr<CellInterface> cell = std::make_unique<Cell>();
    cell->Set(std::move(text));
    return cell;
}

}  // namespace

void Sheet::SetCell(Position position, std::string text) {
    CheckIfPositionValid(position);

    if (auto* cell = GetCell(position)) {
        dynamic_cast<Cell*>(cell)->Set(std::move(text));
    } else {
        ResizeStorage(position);
        data_.at(position.row).at(position.col) = CreateNewCell(text);
    }
}

const CellInterface* Sheet::GetCell(Position position) const {
    return const_cast<Sheet*>(this)->GetCell(position);
}

CellInterface* Sheet::GetCell(Position position) {
    CheckIfPositionValid(position);

    if (position.row >= data_.size())
        return nullptr;
    if (position.col >= data_.at(position.row).size())
        return nullptr;

    return data_.at(position.row).at(position.col).get();
}

void Sheet::ClearCell(Position position) {
    CheckIfPositionValid(position);

    if (GetCell(position))
        data_.at(position.row).at(position.col).release();
}

Size Sheet::GetPrintableSize() const {
    auto get_biggest_id = [](const std::vector<std::unique_ptr<CellInterface>>& col) -> size_t {
        for (int id = col.size() - 1; id >= 0; --id)
            if (col.at(id))
                return id;
        return 0;
    };

    for (int row_id = data_.size() - 1; row_id >= 0; --row_id) {
        int col_id = get_biggest_id(data_.at(row_id));
        if (col_id != 0)
            return {row_id, col_id};
    }

    return {0, 0};
}

void Sheet::PrintValues(std::ostream& output) const {
    Print(output, [](CellInterface* cell) { return cell->GetValue(); });
}
void Sheet::PrintTexts(std::ostream& output) const {
    Print(output, [](CellInterface* cell) { return cell->GetText(); });
}

void Sheet::ResizeStorage(Position position) {
    if (position.row >= data_.size())
        data_.resize(position.row + 1);

    if (position.col >= data_.at(position.row).size())
        data_.at(position.row).resize(position.col + 1);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
