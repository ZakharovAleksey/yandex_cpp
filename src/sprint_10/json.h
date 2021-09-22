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

// All methods should throw this error while JSON parsing
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private std::variant<std::nullptr_t, bool, int, double, std::string, Dict, Array> {
public:  // Using
    // Make available all constructors of the parent variant class
    using variant::variant;
    using Value = variant;

public:  // Methods
    bool IsNull() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsMap() const;

    const Value& GetValue() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

public:  // Operators
    friend bool operator==(const Node& left, const Node& right);
    friend bool operator!=(const Node& left, const Node& right);
};

class Document {
public:  // Constructor
    explicit Document(Node root);

public:  // Methods
    [[nodiscard]] const Node& GetRoot() const;

public:  // Operators
    friend bool operator==(const Document& left, const Document& right);
    friend bool operator!=(const Document& left, const Document& right);

private:  // Fields
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json