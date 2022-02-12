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

IndentParser::IndentParser(std::istream& input) : input_(input) {
    ParseNextLine();
}

int IndentParser::GetIndent() const {
    return indent_;
}

std::stringstream& IndentParser::GetParsedStream() {
    return line_;
}

bool IndentParser::IsInputStreamEmpty() const {
    return !static_cast<bool>(input_);
}

bool IndentParser::EmptyLine(std::string_view line) const {
    if (line.empty())
        return true;

    size_t position = line.find_first_not_of(' ');
    return line[position] == '#' || (position > 0 && position == (line.size() - 1));
}

void IndentParser::ParseNextLine() {
    std::string line;

    while (getline(input_, line)) {
        if (EmptyLine(line))
            continue;

        indent_ = line.find_first_not_of(' ', 0);
        indent_ = indent_ >= 2 ? indent_ / 2 : indent_;

        line = line.substr(indent_);
        line_ = std::stringstream{line};
        return;
    }
    // On empty input we should close all opened indents
    indent_ = 0;
}

Lexer::Lexer(std::istream& input) : indent_reader_(input), current_(NextTokenImpl()) {}

const Token& Lexer::CurrentToken() const {
    return current_;
}

Token Lexer::NextToken() {
    current_ = NextTokenImpl();
    return CurrentToken();
}

Token Lexer::NextTokenImpl() {
    using namespace parse::token_type;

    if (indent_ > indent_reader_.GetIndent()) {
        --indent_;
        return Dedent({});
    } else if (indent_ < indent_reader_.GetIndent()) {
        ++indent_;
        return Indent({});
    }

    char symbol;
    auto& ss = indent_reader_.GetParsedStream();

    while (ss.get(symbol)) {
        if (symbol == ' ')
            continue;

        if (symbol == '#') {
            ss.ignore(numeric_limits<streamsize>::max(), '\n');
            indent_reader_.ParseNextLine();
            return Newline({});
        }

        if (isdigit(symbol)) {
            ss.unget();
            return ReadNumber(ss);
        } else if (isprint(symbol) || isspace(symbol)) {
            switch (symbol) {
                case '=':
                case '!':
                case '<':
                case '>': {
                    if (ss.peek() == '=') {
                        ss.get();
                        if (symbol == '=') {
                            return Eq({});
                        } else if (symbol == '!') {
                            return NotEq({});
                        } else if (symbol == '<') {
                            return LessOrEq({});
                        } else {
                            return GreaterOrEq({});
                        }
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
                    return Char({symbol});
                }
                case '\'':
                case '\"': {
                    return ReadString(ss, symbol);
                }
                default: {
                    ss.unget();
                    return ReadId(ss);
                }
            }
        }
    }

    if (indent_reader_.IsInputStreamEmpty())
        return Eof{};

    indent_reader_.ParseNextLine();
    return Newline{};
}

Token Lexer::ReadId(std::stringstream& ss) {
    using namespace std::string_literals;
    using namespace parse::token_type;

    std::string identifier;
    char symbol;

    while (ss.get(symbol)) {
        if ((isspace(symbol) || symbol == ' ') || (ispunct(symbol) && symbol != '_')) {
            if (ispunct(symbol) && symbol != '_') {
                ss.unget();
            }
            break;
        }
        identifier += symbol;
    }

    if (identifier == "class"s) {
        return Class({});
    } else if (identifier == "return"s) {
        return Return({});
    } else if (identifier == "if"s) {
        return If({});
    } else if (identifier == "else"s) {
        return Else({});
    } else if (identifier == "def"s) {
        return Def({});
    } else if (identifier == "print"s) {
        return Print({});
    } else if (identifier == "or"s) {
        return Or({});
    } else if (identifier == "None"s) {
        return None({});
    } else if (identifier == "and"s) {
        return And({});
    } else if (identifier == "not"s) {
        return Not({});
    } else if (identifier == "True"s) {
        return True({});
    } else if (identifier == "False"s) {
        return False({});
    } else {
        return Id({identifier});
    }
}

Token Lexer::ReadString(std::stringstream& ss, char last_symbol) {
    using namespace token_type;
    std::string result;
    char symbol;

    while (ss.get(symbol)) {
        if (symbol == last_symbol)
            break;

        if (symbol == '\\') {
            ss.get(symbol);
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

    return String({result});
}

Token Lexer::ReadNumber(std::stringstream& ss) {
    using namespace token_type;

    int value;
    ss >> value;
    return Number({value});
}

}  // namespace parse