#include "json.h"

#include <cctype>
#include <unordered_map>

using namespace std;

namespace json {

namespace {

using Number = std::variant<int, double>;

static const std::unordered_map<char, char> kEscapeSymbolsDirectOrder{
    {'\n', 'n'}, {'\t', 't'}, {'\r', 'r'}, {'\\', '\\'}, {'\"', '"'}};

static std::unordered_map<char, char> kEscapeSymbolsReversedOrder{
    {'\\', '\\'}, {'"', '"'}, {'n', '\n'}, {'t', '\t'}, {'r', '\r'}};

// Max size between "false"s.size() and "true"s.size()
static constexpr int kMaxBooleanStringSize{5};

struct NodeContainerPrinter {
    std::ostream& out;

    void operator()(std::nullptr_t /* value */) const {
        out << "null"s;
    }

    void operator()(bool value) const {
        out << std::boolalpha << value;
    }

    void operator()(int value) const {
        out << value;
    }

    void operator()(double value) const {
        out << value;
    }

    void operator()(const std::string& value) const {
        out << "\"";

        for (const char symbol : value) {
            if (kEscapeSymbolsDirectOrder.count(symbol)) {
                out << '\\' << kEscapeSymbolsDirectOrder.at(symbol);
            } else {
                out << symbol;
            }
        }

        out << '\"';
    }

    void operator()(const Dict& map) const {
        out << '{';
        int id{0};
        for (const auto& [key, value] : map) {
            if (id++ != 0)
                out << ", "s;
            out << '"' << key << "\":";
            std::visit(NodeContainerPrinter{out}, value.AsPureNodeContainer());
        }
        out << '}';
    }

    void operator()(const Array& array) const {
        out << '[';

        int id{0};
        for (const auto& value : array) {
            if (id++ != 0)
                out << ", "s;
            std::visit(NodeContainerPrinter{out}, value.AsPureNodeContainer());
        }

        out << ']';
    }
};

/* Load methods */

Node LoadNode(istream& input);

Node LoadNull(istream& input) {
    static const std::string expected{"null"s};
    std::string result;
    result.reserve(4);

    for (char symbol; input >> symbol && result.size() != expected.size();) {
        result.push_back(symbol);
        if (result == expected)
            return Node{};
    }

    throw ParsingError("Incorrect null value: expected \"null\" string"s);
}

Node LoadBool(std::istream& input) {
    std::string result;
    result.reserve(kMaxBooleanStringSize);

    int id{0};
    for (char symbol; input >> symbol && id++ != kMaxBooleanStringSize;) {
        result.push_back(symbol);
        if (result == "true"s)
            return Node{true};
        if (result == "false"s)
            return Node{false};
    }

    throw ParsingError("Incorrect boolean value: expected \"true\" or \"false\" strings"s);
}

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.emplace_back(LoadNode(input));
    }

    if (result.empty())
        throw ParsingError("Incorrect array parsing. Array could not be empty"s);

    return Node(std::move(result));
}

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    input >> std::noskipws;
    std::string result;

    char current{' '};
    char next{' '};

    while (input.get(current)) {
        next = static_cast<char>(input.peek());

        // If " is not a part of \" symbol - this is the end of the line
        if (current == '"')
            break;

        // If string starts with '\\' -> this is an escape symbol
        if (current == '\\') {
            input.get(next);

            if (kEscapeSymbolsReversedOrder.count(next) > 0)
                result += kEscapeSymbolsReversedOrder.at(next);

        } else {
            result += current;
        }
    }

    input >> std::skipws;
    if (result.empty() || current != '"')
        throw ParsingError("Empty string as an input");

    return Node(std::move(result));
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if (result.empty())
        throw ParsingError("Incorrect dictionary parsing. Dict could not be empty"s);

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else {
        input.putback(c);
        auto value = LoadNumber(input);
        return std::holds_alternative<int>(value) ? Node{std::get<int>(value)} : Node{std::get<double>(value)};
    }
}

}  // namespace

/* Constructors */

Node::Node(bool value) : data_(value) {}
Node::Node(std::nullptr_t /* value*/) : Node() {}
Node::Node(int value) : data_(value) {}
Node::Node(double value) : data_(value) {}
Node::Node(std::string value) : data_(std::move(value)) {}
Node::Node(Dict map) : data_(std::move(map)) {}
Node::Node(Array array) : data_(std::move(array)) {}

/* Is-like methods */

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(data_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(data_);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(data_);
}
bool Node::IsDouble() const {
    return std::holds_alternative<double>(data_) || std::holds_alternative<int>(data_);
}
bool Node::IsPureDouble() const {
    double integral_part{0.};
    return !std::holds_alternative<int>(data_) && std::fmod(std::get<double>(data_), integral_part) != 0.;
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(data_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(data_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(data_);
}

/* As-like methods */

const NodeContainer& Node::AsPureNodeContainer() const {
    return data_;
}

const bool& Node::AsBool() const {
    if (auto* value = std::get_if<bool>(&data_))
        return *value;
    throw std::logic_error("Impossible to parse node as Boolean"s);
}

int Node::AsInt() const {
    if (auto* value = std::get_if<int>(&data_))
        return *value;
    throw std::logic_error("Impossible to parse node as Int "s);
}

double Node::AsDouble() const {
    if (auto* value = std::get_if<double>(&data_))
        return *value;

    if (auto* value = std::get_if<int>(&data_))
        return static_cast<double>(*value);

    throw std::logic_error("Impossible to parse node as Double "s);
}

const std::string& Node::AsString() const {
    if (auto* value = std::get_if<std::string>(&data_))
        return *value;
    throw std::logic_error("Impossible to parse node as String"s);
}

const Array& Node::AsArray() const {
    if (auto* value = std::get_if<Array>(&data_))
        return *value;
    throw std::logic_error("Impossible to parse node as Array"s);
}

const Dict& Node::AsMap() const {
    if (auto* value = std::get_if<Dict>(&data_))
        return *value;
    throw std::logic_error("Impossible to parse node as Dict"s);
}

/* Operators */

bool operator==(const Node& left, const Node& right) {
    return left.data_ == right.data_;
}

Document::Document(Node root) : root_(move(root)) {}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    std::visit(NodeContainerPrinter{output}, doc.GetRoot().AsPureNodeContainer());

    // Реализуйте функцию самостоятельно
}

}  // namespace json