//
// Created by azakharov on 4/27/2021.
//

#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <tuple>
#include <vector>

namespace sprint_6::data_structures {


template <typename Type>
class SingleLinkedList {
public:  // Types
    struct Node {
        Node() = default;
        Node(const Type& val, Node* next) : value(val), next_node(next) {}
        Type value{Type()};
        Node* next_node = nullptr;
    };

public:  // Constructor
    SingleLinkedList() = default;

    ~SingleLinkedList() {
        Clear();
    }

    SingleLinkedList(std::initializer_list<Type> values) {
        for (auto iter = std::rbegin(values); iter != std::rend(values); ++iter)
            PushFront(*iter);
    }

    SingleLinkedList(const SingleLinkedList& other) {
        SingleLinkedList<Type> temporary;

        auto other_node = &other.head_;
        auto temporary_node = &temporary.head_;
        while (other_node->next_node) {
            temporary_node->next_node = new Node(other_node->next_node->value, nullptr);

            other_node = other_node->next_node;
            temporary_node = temporary_node->next_node;
        }
        temporary.size_ = other.size_;

        swap(temporary);
    }

    SingleLinkedList& operator=(const SingleLinkedList& other) {
        if (this != &other) {
            auto temporary(other);
            swap(temporary);
        }
        return *this;
    }

    void swap(SingleLinkedList& other) noexcept {
        std::swap(head_.next_node, other.head_.next_node);
        std::swap(size_, other.size_);
    }

public:  // Methods
    void PushFront(const Type& value) {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }

    void Clear() noexcept {
        while (head_.next_node) {
            Node* parent = head_.next_node->next_node;
            delete head_.next_node;
            head_.next_node = parent;
        }
        size_ = 0u;
    }

    [[nodiscard]] int GetSize() const noexcept {
        return size_;
    }

    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0u;
    }

private:
    template <typename ValueType>
    class BasicIterator {
        friend class SingleLinkedList;

    public:  // Types
        using iterator_category = std::forward_iterator_tag;
        using value_type = Type;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

    public:  // Constructors
        BasicIterator() = default;

        BasicIterator(const BasicIterator<Type>& other) noexcept : node_(other.node_) {}

    public:  // Methods
        BasicIterator& operator=(const BasicIterator& rhs) = default;

        [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept {
            return !(*this == rhs);
        }

        BasicIterator& operator++() noexcept {
            node_ = node_->next_node;
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            auto old_value(*this);
            ++(*this);
            return old_value;
        }

        [[nodiscard]] reference operator*() const noexcept {
            return node_->value;
        }

        [[nodiscard]] pointer operator->() const noexcept {
            return &node_->value;
        }

    private:  // Constructor
        explicit BasicIterator(Node* node) : node_(node) {}

    private:  // Fields
        Node* node_{nullptr};
    };

public:
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;

    using Iterator = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;

    [[nodiscard]] Iterator begin() noexcept {
        return Iterator(head_.next_node);
    }

    [[nodiscard]] Iterator end() noexcept {
        auto current_node = &head_;
        while (current_node->next_node)
            current_node = current_node->next_node;
        return Iterator(current_node->next_node);
    }

    [[nodiscard]] ConstIterator begin() const noexcept {
        return ConstIterator(head_.next_node);
    }

    [[nodiscard]] ConstIterator end() const noexcept {
        auto current_node = &head_;
        while (current_node->next_node)
            current_node = current_node->next_node;
        return ConstIterator(current_node->next_node);
    }

    [[nodiscard]] ConstIterator cbegin() const noexcept {
        return ConstIterator(head_.next_node);
    }

    [[nodiscard]] ConstIterator cend() const noexcept {
        auto current_node = &head_;
        while (current_node->next_node)
            current_node = current_node->next_node;
        return ConstIterator(current_node->next_node);
    }

    [[nodiscard]] Iterator before_begin() noexcept {
        return Iterator(&head_);
    }

    [[nodiscard]] ConstIterator cbefore_begin() const noexcept {
        auto result = &const_cast<SingleLinkedList<Type>&>(*this).head_;
        return ConstIterator(result);
    }

    [[nodiscard]] ConstIterator before_begin() const noexcept {
        auto result = &const_cast<SingleLinkedList<Type>&>(*this).head_;
        return ConstIterator(result);
    }

    Iterator InsertAfter(ConstIterator position, const Type& value) {
        Node* position_after_insertion = position.node_->next_node;
        position.node_->next_node = new Node(value, position_after_insertion);
        ++size_;

        return Iterator(position.node_->next_node);
    }

    void PopFront() noexcept {
        if (Node* front_node = head_.next_node) {
            // Change pointers from: 1 -> 2 -> 3 to: 1 -> 3 and remove node 2
            Node* new_front_node = front_node->next_node;
            delete head_.next_node;

            head_.next_node = new_front_node;
            --size_;
        }
    }

    Iterator EraseAfter(ConstIterator position) noexcept {
        Node* position_for_removal = position.node_->next_node;
        Node* position_after_removal = position_for_removal->next_node;

        delete position_for_removal;
        position.node_->next_node = position_after_removal;
        --size_;

        return Iterator(position_after_removal);
    }

private:
    Node head_;
    int size_{0u};
};

template <typename Type>
void swap(SingleLinkedList<Type>& lhs, SingleLinkedList<Type>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return (lhs < rhs) || (lhs == rhs);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& lhs, const SingleLinkedList<Type>& rhs) {
    return (rhs < lhs) || (rhs == lhs);
}

template <typename Type>
std::ostream& operator<<(std::ostream& os, const SingleLinkedList<Type>& list) {
    os << "{ ";
    std::copy(list.begin(), list.end(), std::ostream_iterator<Type>(os, " "));
    os << " }";
    return os;
}

}  // namespace sprint_6::data_structures