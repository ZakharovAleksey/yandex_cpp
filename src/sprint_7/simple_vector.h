//
// Created by azakharov on 5/25/2021.
//

#pragma once

#include <algorithm>
#include <initializer_list>
#include <utility>

#include "array_ptr.h"

using namespace std::literals;

struct ReserveProxyObject {
    size_t capacity_to_reserve;
    explicit ReserveProxyObject(size_t capacity) : capacity_to_reserve(capacity) {}
};

ReserveProxyObject Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObject(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:  // Types
    using Iterator = Type*;
    using ConstIterator = const Type*;

public:  // Constructor & assigment operator
    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : size_(size), capacity_(size_), data_(size) {
        std::fill(data_.Get(), data_.Get() + size_, Type());
    }

    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size_), data_(size_) {
        std::fill(data_.Get(), data_.Get() + size_, value);
    }

    SimpleVector(std::initializer_list<Type> values_list) {
        Assign(values_list.begin(), values_list.end());
    }

    SimpleVector(const SimpleVector& other) {
        Assign(other.begin(), other.end());
    }

    SimpleVector(SimpleVector&& other) noexcept : size_(other.size_), capacity_(other.capacity_) {
        other.size_ = 0u;
        other.capacity_ = 0u;
        data_.swap(other.data_);
        [[maybe_unused]] auto pointer = other.data_.Release();
    }

    explicit SimpleVector(ReserveProxyObject wrapper) {
        capacity_ = wrapper.capacity_to_reserve;
        Reserve(capacity_);
    }

    SimpleVector& operator=(const SimpleVector& other) {
        if (this != &other) {
            auto temporary(other);
            swap(temporary);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& other) noexcept {
        if (this != &other) {
            size_ = other.size_;
            other.size_ = 0u;

            capacity_ = other.capacity_;
            other.capacity_ = 0u;

            data_.swap(other.data_);
            [[maybe_unused]] auto pointer = other.data_.Release();
        }
        return *this;
    }

public:  // Methods
    void PushBack(const Type& value) {
        if (size_ >= capacity_) {
            // Allocate memory
            size_t new_capacity = capacity_ == 0 ? 1 : 2u * capacity_;
            ArrayPtr<Type> new_data(new_capacity);

            // Copy elements
            std::fill(new_data.Get(), new_data.Get() + new_capacity, Type());
            std::copy(data_.Get(), data_.Get() + size_, new_data.Get());

            // Swap data
            data_.swap(new_data);
            capacity_ = new_capacity;
        }
        ++size_;
        data_[size_ - 1] = value;
    }

    void PushBack(Type&& value) {
        if (size_ >= capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : 2u * capacity_;
            ArrayPtr<Type> new_data(new_capacity);

            std::generate(new_data.Get(), new_data.Get() + new_capacity, []() { return Type(); });
            std::move(data_.Get(), data_.Get() + size_, new_data.Get());

            data_.swap(new_data);
            capacity_ = new_capacity;
        }
        ++size_;
        data_[size_ - 1] = std::move(value);
    }

    void PopBack() noexcept {
        data_[size_ - 1] = Type();
        --size_;
    }

    Iterator Insert(ConstIterator position, const Type& value) {
        size_t new_element_index = position - data_.Get();

        if (size_ >= capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : 2u * capacity_;

            ArrayPtr<Type> new_data(new_capacity);
            std::fill(new_data.Get(), new_data.Get() + new_capacity, Type());

            std::copy(data_.Get(), const_cast<Iterator>(position), new_data.Get());
            new_data[new_element_index] = value;
            std::copy(const_cast<Iterator>(position), data_.Get() + size_, new_data.Get() + new_element_index + 1u);

            data_.swap(new_data);
            capacity_ = new_capacity;
        } else {
            std::copy_backward(const_cast<Iterator>(position), data_.Get() + size_, data_.Get() + size_ + 1u);
            data_[new_element_index] = value;
        }
        ++size_;
        return Iterator(data_.Get() + new_element_index);
    }

    Iterator Insert(ConstIterator position, Type&& value) {
        size_t new_element_index = position - data_.Get();

        if (size_ >= capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : 2u * capacity_;

            ArrayPtr<Type> new_data(new_capacity);
            std::generate(new_data.Get(), new_data.Get() + new_capacity, []() { return Type(); });

            std::move(data_.Get(), const_cast<Iterator>(position), new_data.Get());
            new_data[new_element_index] = std::move(value);
            std::move(const_cast<Iterator>(position), data_.Get() + size_, new_data.Get() + new_element_index + 1u);

            data_.swap(new_data);
            capacity_ = new_capacity;
        } else {
            std::move_backward(const_cast<Iterator>(position), data_.Get() + size_, data_.Get() + size_ + 1u);
            data_[new_element_index] = std::move(value);
        }
        ++size_;
        return Iterator(data_.Get() + new_element_index);
    }

    Iterator Erase(ConstIterator position) {
        size_t index = position - data_.Get();
        for (auto current_position = (Iterator)position; current_position != end() - 1; ++current_position)
            *current_position = std::move(*(current_position + 1));
        data_[size_ - 1] = Type();
        --size_;

        return Iterator(data_.Get() + index);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_data(new_capacity);
            std::fill(new_data.Get(), new_data.Get() + capacity_, Type());
            std::copy(data_.Get(), data_.Get() + size_, new_data.Get());

            data_.swap(new_data);
            capacity_ = new_capacity;
        }
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            auto new_data = ArrayPtr<Type>(new_size);
            std::fill(new_data.Get(), new_data.Get() + new_size, Type());
            std::copy(data_.Get(), data_.Get() + size_, new_data.Get());

            [[maybe_unused]] auto unused_pointer = data_.Release();
            data_.swap(new_data);
            capacity_ = new_size;
        } else if (new_size > size_)
            std::fill(data_.Get() + size_, data_.Get() + new_size, Type());
        size_ = new_size;
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void swap(SimpleVector<Type>& other) noexcept {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    [[nodiscard]] size_t GetSize() const noexcept {
        return size_;
    }

    [[nodiscard]] size_t GetCapacity() const noexcept {
        return capacity_;
    }

    [[nodiscard]] bool IsEmpty() const noexcept {
        return size_ == 0u;
    }

    Type& operator[](size_t index) noexcept {
        return data_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return data_[index];
    }

    Type& At(size_t index) {
        if (index >= size_)
            throw std::out_of_range("Index of array is out of range"s);

        return data_[index];
    }

    [[nodiscard]] const Type& At(size_t index) const {
        if (index >= size_)
            throw std::out_of_range("Index of array is out of range"s);

        return data_[index];
    }

    Iterator begin() noexcept {
        return data_.Get();
    }

    Iterator end() noexcept {
        return data_.Get() + size_;
    }

    [[nodiscard]] ConstIterator begin() const noexcept {
        return data_.Get();
    }

    [[nodiscard]] ConstIterator end() const noexcept {
        return data_.Get() + size_;
    }

    [[nodiscard]] ConstIterator cbegin() const noexcept {
        return begin();
    }

    [[nodiscard]] ConstIterator cend() const noexcept {
        return end();
    }

private:  // Methods
    template <class InputIterator>
    void Assign(InputIterator begin, InputIterator end) {
        size_t size = std::distance(begin, end);
        SimpleVector<Type> temporary(size);
        std::copy(begin, end, temporary.begin());
        swap(temporary);
    }

private:
    size_t size_{0u};
    size_t capacity_{0u};
    ArrayPtr<Type> data_;
};

template <typename Type>
bool operator==(const SimpleVector<Type>& left, const SimpleVector<Type>& right) {
    return left.GetSize() == right.GetSize() && std::equal(left.begin(), left.end(), right.begin());
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& left, const SimpleVector<Type>& right) {
    return !(left == right);
}

template <typename Type>
bool operator<(const SimpleVector<Type>& left, const SimpleVector<Type>& right) {
    return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& left, const SimpleVector<Type>& right) {
    return (left < right) || (left == right);
}

template <typename Type>
bool operator>(const SimpleVector<Type>& left, const SimpleVector<Type>& right) {
    return right < left;
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& left, const SimpleVector<Type>& right) {
    return (right < left) || (right == left);
}