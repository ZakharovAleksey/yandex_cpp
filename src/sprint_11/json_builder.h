#pragma once

#include <memory>
#include <optional>
#include <queue>

#include "json.h"

namespace json_11 {

class Builder {
public:  // Constructor
    Builder() = default;

public:  // Methods
    Builder& Key(std::string key);
    Builder& Value(Node::Value v);

    Builder& StartDict();
    Builder& EndDict();

    Builder& StartArray();
    Builder& EndArray();

    const Node& Build() const;

private:  // Methods
    [[nodiscard]] bool CouldAddNode() const;

    void AddNode(Node top_node);

private:  // Fields
    Node root_{nullptr};
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};
}  // namespace json_11