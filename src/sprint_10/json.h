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
    bool IsNull() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsMap() const;

    const NodeContainer& AsPureNodeContainer() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

public:  // Operators
    friend bool operator==(const Node& left, const Node& right);
    friend bool operator!=(const Node& left, const Node& right);

private:  // Fields
    NodeContainer data_;
};

class Document {
public:  // Constructor
    explicit Document(Node root);

public:  // Methods
    [[nodiscard]] const Node& GetRoot() const;

public:  // Operators
    friend bool operator==(const Document& left, const Document& right);
    friend bool operator!=(const Document& left, const Document& right);

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json