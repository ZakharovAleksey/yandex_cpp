#include <cctype>
#include <regex>
#include <sstream>
#include <unordered_map>

#include "common.h"

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

namespace {

int ColumnStringToId(std::string_view column_str) {
    int index{0};
    int pow{1};

    for (int id = static_cast<int>(column_str.length()) - 1; id >= 0; --id) {
        index += ((column_str[id] - 'A') + 1) * pow;
        pow *= LETTERS;
    }

    return index - 1;
}

}  // namespace

bool Position::operator==(const Position rhs) const {
    return std::make_tuple(row, col) == std::make_tuple(rhs.row, rhs.col);
}

bool Position::operator<(const Position rhs) const {
    return std::make_tuple(row, col) < std::make_tuple(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return col >= 0 && col < Position::MAX_COLS && row >= 0 && row < Position::MAX_ROWS;
}

std::string Position::ToString() const {
    std::string result;

    if (!IsValid())
        return result;

    int column_value{col + 1};
    while (column_value > 0) {
        int modulo = (column_value - 1) % LETTERS;
        result.insert(result.begin(), static_cast<char>(static_cast<int>('A') + modulo));

        column_value = (column_value - modulo) / 26;
    }

    return result + std::to_string(row + 1);
}

Position Position::FromString(std::string_view input) {
    using sv_match = std::match_results<std::string_view::const_iterator>;

    static const std::regex kPositionPattern("^([A-Z]{1,3})([1-9][0-9]{0,4})$");

    sv_match match;
    Position result;

    if (std::regex_match(input.cbegin(), input.cend(), match, kPositionPattern)) {
        size_t offset = input.find_first_of("123456789");

        result.col = ColumnStringToId(input.substr(0, offset));
        result.row = std::stoi(std::string(input.substr(offset, input.length() - offset))) - 1;

        return result;
    }

    return Position::NONE;
}