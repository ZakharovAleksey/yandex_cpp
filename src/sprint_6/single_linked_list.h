//
// Created by azakharov on 4/27/2021.
//

#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>

namespace sprint_6::data_structures {

template <typename Type>
class SingleLinkedList {
private:  // Nested structures and classes
    struct Node {
        Node() = default;
        Node(const Type& value, Node* next_node) : value(value), next_node(next_node) {}
        Type value{Type()};
        Node* next_node = nullptr;
    };

    template <typename ValueType>
    class BasicIterator {
        friend class SingleLinkedList;

    public:  // Types
        using iterator_category = std::forward_iterator_tag;
        using value_type = Type;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

    public:  // Constructors and assigment operator
        BasicIterator() = default;

        BasicIterator(const BasicIterator<Type>& other) noexcept : node_(other.node_) {}

        BasicIterator& operator=(const BasicIterator& other) = default;

    public:  // Methods
        [[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept {
            return node_ == rhs.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<const Type>& right) const noexcept {
            return !(*this == right);
        }

        [[nodiscard]] bool operator==(const BasicIterator<Type>& right) const noexcept {
            return node_ == right.node_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator<Type>& right) const noexcept {
            return !(*this == right);
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

public:  // Types
    using value_type = Type;
    using reference = value_type&;
    using const_reference = const value_type&;

    using Iterator = BasicIterator<Type>;
    using ConstantIterator = BasicIterator<const Type>;

public:  // Constructor and assigment operator
    SingleLinkedList() = default;

    SingleLinkedList(std::initializer_list<Type> values) {
        Assign(values.begin(), values.end());
    }

    SingleLinkedList(const SingleLinkedList& other) {
        Assign(other.begin(), other.end());
    }

    SingleLinkedList& operator=(const SingleLinkedList& other) {
        if (this != &other) {
            auto temporary(other);
            swap(temporary);
        }
        return *this;
    }

public:  // Destructor
    ~SingleLinkedList() {
        Clear();
    }

public:  // Methods
    void swap(SingleLinkedList& other) noexcept {
        std::swap(head_.next_node, other.head_.next_node);
        std::swap(size_, other.size_);
    }

    void PushFront(const Type& value) {
        head_.next_node = new Node(value, head_.next_node);
        ++size_;
    }

    void Clear() noexcept {
        while (head_.next_node) {
            Node* node_after_next = head_.next_node->next_node;
            delete head_.next_node;
            head_.next_node = node_after_next;
        }
        size_ = 0u;
    }

    Iterator InsertAfter(ConstantIterator position, const Type& value) {
        assert(position.node_ != nullptr && "Could not insert element after nullptr node");

        Node* position_after_insertion = position.node_->next_node;
        position.node_->next_node = new Node(value, position_after_insertion);
        ++size_;

        return Iterator(position.node_->next_node);
    }

    void PopFront() noexcept {
        assert(!IsEmpty() && "Could not remove first element for the empty list");

        Node* front_node = head_.next_node;
        // Change pointers order from: 1 -> 2 -> 3 to: 1 -> 3 and remove node 2
        Node* new_front_node = front_node->next_node;
        delete head_.next_node;

        head_.next_node = new_front_node;
        --size_;
    }

    Iterator EraseAfter(ConstantIterator position) noexcept {
        assert(!IsEmpty() && "Could not erase elements from the empty list");
        assert(position.node_ != nullptr && "Could not erase element after nullptr node");

        Node* position_for_removal = position.node_->next_node;
        Node* position_after_removal = position_for_removal->next_node;

        delete position_for_removal;
        position.node_->next_node = position_after_removal;
        --size_;

        return Iterator(position_after_removal);
    }

    [[nodiscard]] int GetSize() const noexcept {
        return size_;
    }

    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0u;
    }

    [[nodiscard]] Iterator begin() noexcept {
        return Iterator(head_.next_node);
    }

    [[nodiscard]] Iterator end() noexcept {
        return Iterator(FindNodeAfterLast());
    }

    [[nodiscard]] ConstantIterator begin() const noexcept {
        return ConstantIterator(head_.next_node);
    }

    [[nodiscard]] ConstantIterator end() const noexcept {
        return ConstantIterator(FindNodeAfterLast());
    }

    [[nodiscard]] ConstantIterator cbegin() const noexcept {
        return ConstantIterator(head_.next_node);
    }

    [[nodiscard]] ConstantIterator cend() const noexcept {
        return ConstantIterator(FindNodeAfterLast());
    }

    [[nodiscard]] Iterator before_begin() noexcept {
        return Iterator(&head_);
    }

    [[nodiscard]] ConstantIterator cbefore_begin() const noexcept {
        return ConstantIterator(&const_cast<SingleLinkedList<Type>&>(*this).head_);
    }

    [[nodiscard]] ConstantIterator before_begin() const noexcept {
        return ConstantIterator(cbefore_begin());
    }

private:
    Node* FindNodeAfterLast() const {
        auto current_node = &head_;
        while (current_node->next_node)
            current_node = current_node->next_node;

        return current_node->next_node;
    }

    template <typename InputIterator>
    void Assign(InputIterator begin, InputIterator end) {
        SingleLinkedList<Type> temporary;
        auto temporary_node = &temporary.head_;

        for (auto iterator = begin; iterator != end; ++iterator) {
            temporary_node->next_node = new Node(*iterator, nullptr);
            temporary_node = temporary_node->next_node;
        }

        temporary.size_ = std::distance(begin, end);

        swap(temporary);
    }

private:
    Node head_;
    int size_{0u};
};

template <typename Type>
void swap(SingleLinkedList<Type>& left, SingleLinkedList<Type>& right) noexcept {
    left.swap(right);
}

template <typename Type>
bool operator==(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return left.GetSize() == right.GetSize() && std::equal(left.begin(), left.end(), right.begin());
}

template <typename Type>
bool operator!=(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return !(left == right);
}

template <typename Type>
bool operator<(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
}

template <typename Type>
bool operator<=(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return (left < right) || (left == right);
}

template <typename Type>
bool operator>(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return right < left;
}

template <typename Type>
bool operator>=(const SingleLinkedList<Type>& left, const SingleLinkedList<Type>& right) {
    return (right < left) || (right == left);
}

template <typename Type>
std::ostream& operator<<(std::ostream& os, const SingleLinkedList<Type>& list) {
    os << "{ ";
    std::copy(list.begin(), list.end(), std::ostream_iterator<Type>(os, " "));
    os << " }";
    return os;
}

}  // namespace sprint_6::data_structures