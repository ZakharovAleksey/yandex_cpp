//============================================================================
// Description : Тестирования первого задания: конвертировать между собой
// пользовательский индекс ячейки и её программную позицию.
//============================================================================

#include <iostream>
#include <string>
#include <string_view>

#include "src/sprint_16/common.h"
#include "src/sprint_16/test_runner_p.h"

inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

namespace {
void TestPositionAndStringConversion() {
    auto test_single = [](Position pos, std::string_view str) {
        ASSERT_EQUAL(pos.ToString(), str);
        ASSERT_EQUAL(Position::FromString(str), pos);
    };

    for (int i = 0; i < 25; ++i) {
        test_single(Position{i, i}, char('A' + i) + std::to_string(i + 1));
    }

    test_single(Position{0, 0}, "A1");
    test_single(Position{0, 1}, "B1");
    test_single(Position{0, 25}, "Z1");
    test_single(Position{0, 26}, "AA1");
    test_single(Position{0, 27}, "AB1");
    test_single(Position{0, 51}, "AZ1");
    test_single(Position{0, 52}, "BA1");
    test_single(Position{0, 53}, "BB1");
    test_single(Position{0, 77}, "BZ1");
    test_single(Position{0, 78}, "CA1");
    test_single(Position{0, 701}, "ZZ1");
    test_single(Position{0, 702}, "AAA1");
    test_single(Position{136, 2}, "C137");
    test_single(Position{Position::MAX_ROWS - 1, Position::MAX_COLS - 1}, "XFD16384");
}

void TestPositionToStringInvalid() {
    ASSERT_EQUAL((Position::NONE).ToString(), "");
    ASSERT_EQUAL((Position{-10, 0}).ToString(), "");
    ASSERT_EQUAL((Position{1, -3}).ToString(), "");
}

void TestStringToPositionInvalid() {
    ASSERT(!Position::FromString("").IsValid());
    ASSERT(!Position::FromString("A").IsValid());
    ASSERT(!Position::FromString("1").IsValid());
    ASSERT(!Position::FromString("e2").IsValid());
    ASSERT(!Position::FromString("A0").IsValid());
    ASSERT(!Position::FromString("A-1").IsValid());
    ASSERT(!Position::FromString("A+1").IsValid());
    ASSERT(!Position::FromString("R2D2").IsValid());
    ASSERT(!Position::FromString("C3PO").IsValid());
    ASSERT(!Position::FromString("XFD16385").IsValid());
    ASSERT(!Position::FromString("XFE16384").IsValid());
    ASSERT(!Position::FromString("A1234567890123456789").IsValid());
    ASSERT(!Position::FromString("ABCDEFGHIJKLMNOPQRS8").IsValid());
}
}  // namespace

int LETTERS_1 = 26;
#include <optional>

std::optional<char> GetNextColumnSymbol1(int& current_residue, int pow) {
    auto make_letter = [](int letter_id) -> char { return static_cast<char>(static_cast<int>('A') + letter_id); };

    if (current_residue < pow || pow == 1)
        return (pow == 1) ? std::make_optional(make_letter(current_residue)) : std::nullopt;

    int max_fit_value{0};
    for (int letter_id = 0; letter_id < LETTERS_1; ++letter_id) {
        max_fit_value += pow;

        if (max_fit_value == current_residue) {
            current_residue = 0;
            return std::make_optional(make_letter(letter_id));
        } else if (max_fit_value > current_residue) {
            current_residue = current_residue % (max_fit_value - pow);
            // Use ' - 1' below because at this point we exceed the letter id (52 for input 27)
            return std::make_optional(make_letter(letter_id - 1));
        }
    }

    return std::nullopt;
}

std::string ToString1(int col) {
    std::string result;

    int current_residue = col;
    // String representation: SYMBOL_0 * 26^2 + SYMBOL_1 * 26^1 + SYMBOL_2 + 26^0
    for (const auto& pow : {LETTERS_1 * LETTERS_1, LETTERS_1, 1}) {
        if (auto symbol = GetNextColumnSymbol1(current_residue, pow); symbol.has_value())
            result += symbol.value();
    }

    return result;
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestPositionAndStringConversion);
    RUN_TEST(tr, TestPositionToStringInvalid);
    RUN_TEST(tr, TestStringToPositionInvalid);
    return 0;
}