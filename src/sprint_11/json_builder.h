#pragma once

/*
 * Description: provide an interface for correct JSON file creation
 * TODO: Main changes of sprint-11. For review
 */

#include <memory>
#include <optional>
#include <queue>

#include "json.h"

namespace json {

class Builder;
class DictContext;
class ArrayContext;
class ValueContext;

/* BASE CONTEXT */

/*
 * Contexts below provides interfaces for the correct JSON creation.
 * For example, they do not allow the creation of JSON: {"name", [ } and many others.
 * Idea: create context + add only necessary methods to it, so that when user call for wrong method, he will get a
 * compilation error.
 */

class BaseContext {
public:  // Constructor
    explicit BaseContext(Builder& builder);

protected:  // Fields
    Builder& builder_;
};

class StartContainersContext : public BaseContext {
public:  // Constructor
    explicit StartContainersContext(Builder& builder);

public:  // Fields
    ArrayContext& StartArray();
    DictContext& StartDict();
};

/* JSON CONTEXTS */

class KeyContext : public StartContainersContext {
    /* Methods: Value | StartDict, StartArray */
public:  // Constructor
    explicit KeyContext(Builder& builder);

public:  // Methods
    ValueContext Value(Node::Value value);
};

class ValueContext : public BaseContext {
    /* Methods: Key, EndDict : dict value call*/
public:  // Constructor
    explicit ValueContext(Builder& builder);

public:  // Methods
    KeyContext& Key(std::string key);
    Builder& EndDict();
};

class DictContext : public BaseContext {
    /* Methods: Key, EndDict */
public:  // Constructor
    explicit DictContext(Builder& builder);

public:  // Methods
    KeyContext& Key(std::string key);
    Builder& EndDict();
};

class ArrayContext : public StartContainersContext {
    /* Methods: Value, EndArray | StartDict, StartArray */
public:  // Constructor
    explicit ArrayContext(Builder& builder);

public:  // Methods
    ArrayContext& Value(Node::Value value);
    Builder& EndArray();
};

/* BUILDER */

/// Provides the interface for the correct JSON creation.
/// @throws std::logical_error in case of incorrect attempt of JSON creation
class Builder final : virtual public KeyContext,
                      virtual public ValueContext,
                      virtual public DictContext,
                      virtual public ArrayContext {
public:  // Constructor
    Builder();

public:  // Methods
    KeyContext& Key(std::string key);
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
}  // namespace json