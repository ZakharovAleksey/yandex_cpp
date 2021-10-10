#pragma once

#include <optional>
#include <queue>

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
    mutable bool is_ready_{false};
    std::queue<bool> array_checker;
    std::queue<bool> dict_checker;
    std::optional<std::string> temporary_key{std::nullopt};
    Node root_{nullptr};
    std::vector<Node*> nodes_stack_;
};
}  // namespace json_11