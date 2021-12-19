#include "lexer.h"

#include <algorithm>
#include <limits>

using namespace std;

namespace parse {

bool operator==(const Token& lhs, const Token& rhs) {
    using namespace token_type;

    if (lhs.index() != rhs.index()) {
        return false;
    }
    if (lhs.Is<Char>()) {
        return lhs.As<Char>().value == rhs.As<Char>().value;
    }
    if (lhs.Is<Number>()) {
        return lhs.As<Number>().value == rhs.As<Number>().value;
    }
    if (lhs.Is<String>()) {
        return lhs.As<String>().value == rhs.As<String>().value;
    }
    if (lhs.Is<Id>()) {
        return lhs.As<Id>().value == rhs.As<Id>().value;
    }
    return true;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Token& rhs) {
    using namespace token_type;

// clang-format off
#define VALUED_OUTPUT(type)                                     \
    if (auto p = rhs.TryAs<type>())                             \
        return os << #type << '{' << p->value << '}';           \
// clang-format on

    VALUED_OUTPUT(Number);
    VALUED_OUTPUT(Id);
    VALUED_OUTPUT(String);
    VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

// clang-format off
#define UNVALUED_OUTPUT(type)                                   \
    if (rhs.Is<type>())                                         \
        return os << #type;
    // clang-format on

    UNVALUED_OUTPUT(Class);
    UNVALUED_OUTPUT(Return);
    UNVALUED_OUTPUT(If);
    UNVALUED_OUTPUT(Else);
    UNVALUED_OUTPUT(Def);
    UNVALUED_OUTPUT(Newline);
    UNVALUED_OUTPUT(Print);
    UNVALUED_OUTPUT(Indent);
    UNVALUED_OUTPUT(Dedent);
    UNVALUED_OUTPUT(And);
    UNVALUED_OUTPUT(Or);
    UNVALUED_OUTPUT(Not);
    UNVALUED_OUTPUT(Eq);
    UNVALUED_OUTPUT(NotEq);
    UNVALUED_OUTPUT(LessOrEq);
    UNVALUED_OUTPUT(GreaterOrEq);
    UNVALUED_OUTPUT(None);
    UNVALUED_OUTPUT(True);
    UNVALUED_OUTPUT(False);
    UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

    return os << "Unknown token :("sv;
}

Lexer::Lexer(std::istream& input) {
    using namespace parse;
    using namespace token_type;

    std::string line;
    while (getline(input, line)) {
        if (EmptyLine(line))
            continue;

        SetNewIndent(GetAndCutLineIndent(line));
        std::istringstream ss(line);
        ReadLine(ss);
    }

    SetNewIndent(0);

    tokens_.emplace_back(Eof{});
}

const Token& Lexer::CurrentToken() const {
    return tokens_.at(current_token_id);
}

Token Lexer::NextToken() {
    current_token_id = current_token_id + 1 < tokens_.size() ? current_token_id + 1 : current_token_id;
    return CurrentToken();
}

size_t Lexer::GetAndCutLineIndent(std::string& input) const {
    size_t indent = input.find_first_not_of(' ', 0);
    indent = indent >= 2 ? indent / 2 : indent;

    input = input.substr(indent);
    return indent;
}

void Lexer::SetNewIndent(size_t new_indent) {
    using namespace parse::token_type;

    for (size_t indent = indent_; indent < new_indent; ++indent)
        tokens_.emplace_back(Indent({}));

    for (size_t dedent = indent_; dedent > new_indent; --dedent)
        tokens_.emplace_back(Dedent({}));

    if (indent_ != new_indent)
        indent_ = new_indent;
}

void Lexer::ReadLine(std::istringstream& input) {
    using namespace parse::token_type;

    char symbol;
    bool is_new_line{false};

    while (input.get(symbol)) {
        if (symbol == ' ')
            continue;

        if (symbol == '#') {
            input.ignore(numeric_limits<streamsize>::max(), '\n');
            tokens_.emplace_back(Newline{});
            return;
        }

        is_new_line = true;
        if (isdigit(symbol)) {
            input.unget();
            ReadNumber(input);
        } else if (isprint(symbol) || isspace(symbol)) {
            switch (symbol) {
                case '=':
                case '!':
                case '<':
                case '>': {
                    if (input.peek() == '=') {
                        if (symbol == '=') {
                            tokens_.emplace_back(Eq({}));
                        } else if (symbol == '!') {
                            tokens_.emplace_back(NotEq({}));
                        } else if (symbol == '<') {
                            tokens_.emplace_back(LessOrEq({}));
                        } else {
                            tokens_.emplace_back(GreaterOrEq({}));
                        }
                        input.get();
                        break;
                    }
                    [[fallthrough]];
                }
                case '*':
                case '/':
                case '+':
                case '-':
                case '(':
                case ')':
                case '?':
                case ',':
                case '.':
                case ':':
                case ';':
                case '\t':
                case '\n': {
                    tokens_.emplace_back(Char{symbol});
                    break;
                }
                case '\'':
                case '\"': {
                    ReadString(input, symbol);
                    break;
                }
                default: {
                    input.unget();
                    ReadId(input);
                }
            }
        }
    }

    if (is_new_line)
        tokens_.emplace_back(Newline{});
}

void Lexer::ReadId(std::istringstream& input) {
    using namespace parse::token_type;

    std::string identifier;
    char symbol;

    while (input.get(symbol)) {
        if ((isspace(symbol) || symbol == ' ') || (ispunct(symbol) && symbol != '_')) {
            if (ispunct(symbol) && symbol != '_') {
                input.unget();
            }
            break;
        }
        identifier += symbol;
    }
    if (identifier == "class") {
        tokens_.emplace_back(Class({}));
    } else if (identifier == "return") {
        tokens_.emplace_back(Return({}));
    } else if (identifier == "if") {
        tokens_.emplace_back(If({}));
    } else if (identifier == "else") {
        tokens_.emplace_back(Else({}));
    } else if (identifier == "def") {
        tokens_.emplace_back(Def({}));
    } else if (identifier == "print") {
        tokens_.emplace_back(Print({}));
    } else if (identifier == "or") {
        tokens_.emplace_back(Or({}));
    } else if (identifier == "None") {
        tokens_.emplace_back(None({}));
    } else if (identifier == "and") {
        tokens_.emplace_back(And({}));
    } else if (identifier == "not") {
        tokens_.emplace_back(Not({}));
    } else if (identifier == "True") {
        tokens_.emplace_back(True({}));
    } else if (identifier == "False") {
        tokens_.emplace_back(False({}));
    } else {
        tokens_.emplace_back(Id({identifier}));
    }
}

void Lexer::ReadString(std::istringstream& input, char last_symbol) {
    using namespace token_type;
    std::string result;
    char symbol;

    while (input.get(symbol)) {
        if (symbol == last_symbol)
            break;

        if (symbol == '\\') {
            input.get(symbol);
            switch (symbol) {
                case 'n':
                    symbol = '\n';
                    break;
                case 't':
                    symbol = '\t';
            }
            result += symbol;
        } else {
            result += symbol;
        }
    }

    tokens_.emplace_back(String{result});
}

void Lexer::ReadNumber(std::istringstream& input) {
    using namespace token_type;

    int value;
    input >> value;
    tokens_.emplace_back(Number{value});
}

bool Lexer::EmptyLine(std::string_view line) const {
    if (line.empty())
        return true;

    size_t position = line.find_first_not_of(' ');
    return line[position] == '#' || (position > 0 && position == (line.size() - 1));
}

}  // namespace parse