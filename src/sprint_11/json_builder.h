#pragma once

#include "json.h"

namespace json_11 {
class Builder {
public:  // Constructor
    Builder();

public:  // Methods
    Builder& Key(std::string key);
    Builder& Value(Node::Value value);

    Builder& StartDict();
    Builder& EndDict();

    Builder& StartArray();
    Builder& EndArray();

    const Node& Build() const;

private:  // Fields
    std::string temporary_key;
    Node root_{nullptr};
    std::vector<Node*> nodes_stack_;
};
}  // namespace json_11