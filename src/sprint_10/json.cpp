#include "json.h"

using namespace std;

namespace json {

namespace {

struct NodeContainerPrinter {
    std::ostream& out;

    void operator()(std::nullptr_t /* value */) const {
        out << "null";
    }
    void operator()(bool value) const {}
    void operator()(int value) const {}
    void operator()(double value) const {}
    void operator()(const std::string& value) const {}
    void operator()(const Dict& map) const {}
    void operator()(const Array& array) const {}
};

/* Load methods */

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(move(result));
}

Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }
    return Node(result);
}

Node LoadString(istream& input) {
    string line;
    getline(input, line, '"');
    return Node(move(line));
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

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == 'n') {
        return Node();
    } else if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else {
        input.putback(c);
        return LoadInt(input);
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

/* As-like methods */

const NodeContainer& Node::AsPureNodeContainer() const {
    return data_;
}

const std::string& Node::AsString() const {
    return std::get<std::string>(data_);
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