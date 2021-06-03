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
    using TypeSmartPtr = ArrayPtr<Type>;

public:  // Constructor & assigment operator
    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : size_(size), capacity_(size_), data_(size) {}

    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size_), data_(size_) {
        std::fill(data_.Get(), data_.Get() + size_, value);
    }

    SimpleVector(std::initializer_list<Type> values_list) {
        Assign(values_list.begin(), values_list.end());
    }

    SimpleVector(const SimpleVector& other) {
        Assign(other.begin(), other.end());
    }

    SimpleVector(SimpleVector&& other) noexcept
        : size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)),
          data_(std::move(other.data_)) {}

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
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
            data_ = std::move(other.data_);
        }
        return *this;
    }

public:  // Methods
    void PushBack(const Type& value) {
        size_t new_size = size_ + 1u;
        if (new_size > capacity_) {
            size_t new_capacity = (capacity_ != 0) ? std::max(new_size, 2u * capacity_) : 1;
            ReallocateAndCopyData(new_capacity);
        }
        data_[size_] = value;
        size_ = new_size;
    }

    void PushBack(Type&& value) {
        size_t new_size = size_ + 1u;
        if (new_size > capacity_) {
            size_t new_capacity = (capacity_ != 0) ? std::max(new_size, 2u * capacity_) : 1;

            TypeSmartPtr new_data(new_capacity);
            std::move(data_.Get(), data_.Get() + size_, new_data.Get());
            data_.swap(new_data);

            capacity_ = new_capacity;
        }
        data_[size_] = std::move(value);
        size_ = new_size;
    }

    void PopBack() noexcept {
        data_[size_ - 1] = Type();
        --size_;
    }

    Iterator Insert(ConstIterator position, const Type& value) {
        size_t position_offset = position - data_.Get();
        assert(position_offset <= size_ && "Could not insert to position greater than size of vector");

        size_t new_size = size_ + 1;
        if (new_size <= capacity_) {
            std::copy_backward(const_cast<Iterator>(position), data_.Get() + size_, data_.Get() + size_ + 1u);
            data_[position_offset] = value;
        } else {
            size_t new_capacity = (capacity_ != 0) ? std::max(new_size, 2u * capacity_) : 1;

            TypeSmartPtr new_data(new_capacity);
            std::copy(data_.Get(), const_cast<Iterator>(position), new_data.Get());
            new_data[position_offset] = value;
            std::copy(const_cast<Iterator>(position), data_.Get() + size_, new_data.Get() + position_offset + 1u);

            data_.swap(new_data);

            capacity_ = new_capacity;
        }
        size_ = new_size;

        return Iterator(data_.Get() + position_offset);
    }

    Iterator Insert(ConstIterator position, Type&& value) {
        size_t position_offset = position - data_.Get();
        assert(position_offset <= size_ && "Could not insert to position greater than size of vector");

        size_t new_size = size_ + 1;
        if (new_size <= capacity_) {
            std::move_backward(const_cast<Iterator>(position), data_.Get() + size_, data_.Get() + size_ + 1u);
            data_[position_offset] = std::move(value);
        } else {
            size_t new_capacity = (capacity_ != 0) ? std::max(new_size, 2u * capacity_) : 1;

            TypeSmartPtr new_data(new_capacity);
            std::move(data_.Get(), const_cast<Iterator>(position), new_data.Get());
            new_data[position_offset] = std::move(value);
            std::move(const_cast<Iterator>(position), data_.Get() + size_, new_data.Get() + position_offset + 1u);

            data_.swap(new_data);
            capacity_ = new_capacity;
        }
        size_ = new_size;

        return Iterator(data_.Get() + position_offset);
    }

    Iterator Erase(ConstIterator position) {
        for (auto current_position = (Iterator)position; current_position != end() - 1; ++current_position)
            *current_position = std::move(*(current_position + 1));
        --size_;

        return Iterator(position);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_)
            ReallocateAndCopyData(new_capacity);
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_)
            ReallocateAndCopyData(new_size);
        else if (new_size > size_)
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

    void ReallocateAndCopyData(size_t new_capacity) {
        TypeSmartPtr new_data(new_capacity);
        std::copy(data_.Get(), data_.Get() + size_, new_data.Get());
        data_.swap(new_data);

        capacity_ = new_capacity;
    }

private:
    size_t size_{0u};
    size_t capacity_{0u};
    TypeSmartPtr data_;
};

template <typename Type>
bool operator==(const SimpleVector<Type>& left, const SimpleVector<Type>& right) {
    return (&left != &right) ? left.GetSize() == right.GetSize() && std::equal(left.begin(), left.end(), right.begin())
                             : true;
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