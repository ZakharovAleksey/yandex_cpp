#include "json_builder.h"

namespace json_11 {

Builder::Builder() {
    Node* root_ptr = &root_;
    nodes_stack_.emplace_back(root_ptr);
}

Builder& Builder::Key(std::string key) {
    if (is_ready_ || nodes_stack_.empty())
        throw std::logic_error("");

    Node* top_node = nodes_stack_.back();

    if (top_node->IsDict() && !temporary_key.has_value()) {
        temporary_key = std::move(key);
    } else {
        throw std::logic_error("Incorrect attempt to build key: " + key);
    }

    return *this;
}

Builder& Builder::Value(Node::Value value) {
    if (is_ready_ || nodes_stack_.empty())
        throw std::logic_error("");

    auto* top_node = nodes_stack_.back();

    if (top_node->IsArray()) {
        auto& array = std::get<Array>(top_node->GetValue());
        array.emplace_back(Node{});
        top_node = &array.back();
    } else if (top_node->IsDict()) {
        if (!temporary_key.has_value())
            throw std::logic_error("Could not Value() for dict without key");

        auto& dict = std::get<Dict>(top_node->GetValue());
        auto [position, _] = dict.emplace(*std::move(temporary_key), Node{});
        temporary_key = std::nullopt;
        top_node = &position->second;
    }

    top_node->GetValue() = std::move(value);

    return *this;
}

Builder& Builder::StartDict() {
    if (is_ready_ || nodes_stack_.empty())
        throw std::logic_error("");

    auto* top_node = nodes_stack_.back();

    if (top_node->IsArray()) {
        auto& array = std::get<Array>(top_node->GetValue());
        array.emplace_back(Dict());
        nodes_stack_.emplace_back(&array.back());
    } else if (top_node->IsDict()) {
        if (!temporary_key.has_value())
            throw std::logic_error("Could not StartDict() for dict without key");

        auto& dict = std::get<Dict>(top_node->GetValue());
        auto [position, _] = dict.emplace(*std::move(temporary_key), Dict());
        temporary_key = std::nullopt;
        nodes_stack_.emplace_back(&position->second);
    } else {
        top_node->GetValue() = Dict();
    }

    dict_checker.push(true);
    return *this;
}

Builder& Builder::EndDict() {
    if (is_ready_ || nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
        throw std::logic_error("");

    nodes_stack_.pop_back();
    dict_checker.pop();
    return *this;
}

Builder& Builder::StartArray() {
    if (is_ready_ || nodes_stack_.empty())
        throw std::logic_error("");

    auto* top_node = nodes_stack_.back();

    if (top_node->IsArray()) {
        auto& array = std::get<Array>(top_node->GetValue());
        array.emplace_back(Array());
        nodes_stack_.emplace_back(&array.back());
    } else if (top_node->IsDict()) {
        if (!temporary_key.has_value())
            throw std::logic_error("Could not StartArray() for dict without key");

        auto& dict = std::get<Dict>(top_node->GetValue());
        auto [position, _] = dict.emplace(std::move(*temporary_key), Array());
        temporary_key = std::nullopt;
        nodes_stack_.emplace_back(&position->second);
    } else {
        top_node->GetValue() = Array();
    }

    array_checker.push(true);
    return *this;
}

Builder& Builder::EndArray() {
    if (is_ready_ || nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
        throw std::logic_error("");

    nodes_stack_.pop_back();
    array_checker.pop();
    return *this;
}

const Node& Builder::Build() const {
    if (root_.IsNull() || nodes_stack_.size() > 1 || !dict_checker.empty() || !array_checker.empty())
        throw std::logic_error("");

    is_ready_ = true;

    return root_;
}

}  // namespace json_11
