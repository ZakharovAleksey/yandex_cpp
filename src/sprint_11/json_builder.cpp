#include "json_builder.h"

namespace json_11 {

Builder::Builder() {
    Node* root_ptr = &root_;
    nodes_stack_.emplace_back(root_ptr);
}

Builder& Builder::Key(std::string key) {
    Node* top_node = nodes_stack_.back();

    if (top_node->IsDict()) {
        temporary_key = std::move(key);
    } else {
        throw std::logic_error("Incorrect attempt to build key: " + key);
    }

    return *this;
}

Builder& Builder::Value(Node::Value value) {
    auto* top_node = nodes_stack_.back();

    if (top_node->IsArray()) {
        auto& array = std::get<Array>(top_node->GetValue());
        array.emplace_back(new Node{});
        top_node = &array.back();
    } else if (top_node->IsDict()) {
        auto& dict = std::get<Dict>(top_node->GetValue());
        auto [position, _] = dict.emplace(std::move(temporary_key), Node{});
        top_node = &position->second;
    }

    top_node->GetValue() = std::move(value);

    return *this;
}

Builder& Builder::StartDict() {
    auto* top_node = nodes_stack_.back();

    if (top_node->IsArray()) {
        auto& array = std::get<Array>(top_node->GetValue());
        array.emplace_back(Dict());
        nodes_stack_.emplace_back(&array.back());
    } else if (top_node->IsDict()) {
        auto& dict = std::get<Dict>(top_node->GetValue());
        auto [position, _] = dict.emplace(std::move(temporary_key), Dict());
        nodes_stack_.emplace_back(&position->second);
    } else {
        top_node->GetValue() = Dict();
    }

    return *this;
}

Builder& Builder::EndDict() {
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::StartArray() {
    auto* top_node = nodes_stack_.back();

    if (top_node->IsArray()) {
        auto& array = std::get<Array>(top_node->GetValue());
        array.emplace_back(Array());
        nodes_stack_.emplace_back(&array.back());
    } else if (top_node->IsDict()) {
        auto& dict = std::get<Dict>(top_node->GetValue());
        auto [position, _] = dict.emplace(std::move(temporary_key), Array());
        nodes_stack_.emplace_back(&position->second);
    } else {
        top_node->GetValue() = Array();
    }

    return *this;
}

Builder& Builder::EndArray() {
    nodes_stack_.pop_back();
    return *this;
}

const Node& Builder::Build() const {
    return root_;
}

}  // namespace json_11
