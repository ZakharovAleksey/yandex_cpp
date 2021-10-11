#pragma once

#include <memory>
#include <optional>
#include <queue>

#include "json.h"

namespace json_11 {

class Builder;
class DictContext;
class ArrayContext;

// StartContainersContext

class BaseContext {
public:
    explicit BaseContext(Builder& builder);

protected:
    Builder& builder_;
};

class StartContainersContext : public BaseContext {
public:
    explicit StartContainersContext(Builder& builder);

public:
    ArrayContext& StartArray();
    DictContext& StartDict();
};

class KeyContext {
    // Value | StartDict, StartArray
};

class DictKeyItemContext {
    /* Methods: Key, EndDict : dict value call*/
};

class DictContext : public BaseContext {
    /* Methods: Key, EndDict */
public:
    explicit DictContext(Builder& builder);

public:
    Builder& Key(std::string key);
    Builder& EndDict();
};

class ArrayContext : public StartContainersContext {
    /* Methods: Value, EndArray | StartDict, StartArray */
public:
    explicit ArrayContext(Builder& builder);

public:
    Builder& Value(Node::Value value);
    Builder& EndArray();
};

class Builder final :  // virtual public KeyContext,
                       // virtual public DictKeyItemContext,
                       virtual public DictContext,
                       virtual public ArrayContext {
public:  // Constructor
    Builder();

public:  // Methods
    Builder& Key(std::string key);
    Builder& Value(Node::Value value);

    DictContext& StartDict();
    Builder& EndDict();

    ArrayContext& StartArray();
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