#include "json_builder.h"

namespace json {

/* BASE CONTEXT */

BaseContext::BaseContext(Builder& builder) : builder_(builder) {}

StartContainersContext::StartContainersContext(Builder& builder) : BaseContext(builder) {}

ArrayContext& StartContainersContext::StartArray() {
    return builder_.StartArray();
}

DictContext& StartContainersContext::StartDict() {
    return builder_.StartDict();
}

/* JSON CONTEXTS */

KeyContext::KeyContext(Builder& builder) : StartContainersContext(builder) {}

ValueContext KeyContext::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}

ValueContext::ValueContext(Builder& builder) : BaseContext(builder) {}

KeyContext& ValueContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& ValueContext::EndDict() {
    return builder_.EndDict();
}

DictContext::DictContext(Builder& builder) : BaseContext(builder) {}

KeyContext& DictContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& DictContext::EndDict() {
    return builder_.EndDict();
}

ArrayContext::ArrayContext(Builder& builder) : StartContainersContext(builder) {}

ArrayContext& ArrayContext::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}

Builder& ArrayContext::EndArray() {
    return builder_.EndArray();
}

/* BUILDER */

Builder::Builder() : KeyContext(*this), ValueContext(*this), DictContext(*this), ArrayContext(*this) {}

KeyContext& Builder::Key(std::string key) {
    if (!root_.IsNull() || nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
        throw std::logic_error("Incorrect attempt to add key :" + key);

    nodes_stack_.emplace_back(std::make_unique<Node>(std::move(key)));
    return *this;
}

Builder& Builder::Value(Node::Value value) {
    if (!root_.IsNull() || !CouldAddNode())
        throw std::logic_error("Incorrect attempt to add Value");

    std::visit([this](auto&& v) { nodes_stack_.emplace_back(std::make_unique<Node>(v)); }, value);
    AddNode(*nodes_stack_.back().release());
    return *this;
}

DictContext& Builder::StartDict() {
    if (!root_.IsNull() || !CouldAddNode())
        throw std::logic_error("Incorrect attempt to start Dict()");

    nodes_stack_.emplace_back(std::make_unique<Node>(Dict()));
    return *this;
}

Builder& Builder::EndDict() {
    if (!root_.IsNull() || nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
        throw std::logic_error("Incorrect attempt to end Dict()");

    AddNode(*nodes_stack_.back().release());
    return *this;
}

ArrayContext& Builder::StartArray() {
    if (!root_.IsNull() || !CouldAddNode())
        throw std::logic_error("Incorrect attempt to start Array()");

    nodes_stack_.emplace_back(std::make_unique<Node>(Array()));
    return *this;
}

Builder& Builder::EndArray() {
    if (!root_.IsNull() || nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
        throw std::logic_error("Incorrect attempt to end Array()");

    AddNode(*nodes_stack_.back().release());
    return *this;
}

const Node& Builder::Build() const {
    if (root_.IsNull() || !nodes_stack_.empty())
        throw std::logic_error("Could not build JSON");

    return root_;
}

bool Builder::CouldAddNode() const {
    return nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsString();
}

void Builder::AddNode(Node top_node) {
    nodes_stack_.pop_back();

    if (nodes_stack_.empty()) {
        root_ = top_node;
    } else if (nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(std::move(top_node));
    } else if (nodes_stack_.back()->IsString()) {
        std::string key = std::get<std::string>(nodes_stack_.back()->GetValue());
        nodes_stack_.pop_back();
        std::get<Dict>(nodes_stack_.back()->GetValue()).emplace(std::move(key), std::move(top_node));
    } else {
        // In case we are trying to create an empty containers
        if (top_node.IsDict()) {
            nodes_stack_.back()->GetValue() = Dict();
        } else if (top_node.IsArray()) {
            nodes_stack_.back()->GetValue() = Array();
        }
    }
}

}  // namespace json
