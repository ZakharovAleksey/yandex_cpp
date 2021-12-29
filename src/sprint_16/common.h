#pragma once

#include <sstream>
#include <string>
#include <string_view>

// Позиция ячейки. Индексация с нуля.
struct Position {
public:  // Constants
    static const int MAX_ROWS{16384};
    static const int MAX_COLS{16384};
    static const Position NONE;

public:  // Methods
    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] std::string ToString() const;

    static Position FromString(std::string_view input);

public:  // Operators
    bool operator==(Position other) const;
    bool operator<(Position other) const;

public:  // Fields
    int row{0};
    int col{0};
};