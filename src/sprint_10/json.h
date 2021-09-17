#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using NodeContainer = std::variant<std::nullptr_t, bool, int, double, std::string, Dict, Array>;

// All methods should throw this error while JSON parsing
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:  // Constructors
    Node() = default;

    Node(std::nullptr_t /* value*/);
    Node(bool value);
    Node(int value);
    Node(double value);
    Node(std::string value);
    Node(Dict map);
    Node(Array array);

public:  // Methods
    [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsBool() const;
    [[nodiscard]] bool IsInt() const;
    [[nodiscard]] bool IsDouble() const;
    [[nodiscard]] bool IsPureDouble() const;
    [[nodiscard]] bool IsString() const;
    [[nodiscard]] bool IsArray() const;
    [[nodiscard]] bool IsMap() const;

    [[nodiscard]] const NodeContainer& AsPureNodeContainer() const;
    [[nodiscard]] const bool& AsBool() const;
    [[nodiscard]] int AsInt() const;
    [[nodiscard]] double AsDouble() const;
    [[nodiscard]] const std::string& AsString() const;
    [[nodiscard]] const Array& AsArray() const;
    [[nodiscard]] const Dict& AsMap() const;

    //    const Array& AsArray() const;
    //    const Dict& AsMap() const;
    //    int AsInt() const;

public:
    friend bool operator==(const Node& left, const Node& right);

private:
    NodeContainer data_;
};

class Document {
public:
    explicit Document(Node root);

    [[nodiscard]] const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json