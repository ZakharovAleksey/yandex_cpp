#pragma once

#include <cassert>
#include <cstdlib>
#include <memory>
#include <new>
#include <utility>

template <typename Type>
class RawMemory {
public:  // Constructors
    RawMemory() = default;
    explicit RawMemory(size_t capacity);

public:  // Destructors
    ~RawMemory();

public:  // Operators
    Type* operator+(size_t offset) noexcept;
    const Type* operator+(size_t offset) const noexcept;
    const Type& operator[](size_t index) const noexcept;
    Type& operator[](size_t index) noexcept;

public:  // Methods
    void Swap(RawMemory& other) noexcept;
    const Type* GetAddress() const noexcept;
    [[maybe_unused]] Type* GetAddress() noexcept;
    [[nodiscard]] size_t Capacity() const;

private:  // Methods
    static Type* Allocate(size_t n);
    static void Deallocate(Type* buf) noexcept;

private:  // Fields
    Type* buffer_{nullptr};
    size_t capacity_{0u};
};

template <typename Type>
class Vector {
public:  // Constructor
    Vector() = default;
    explicit Vector(size_t size);
    Vector(const Vector& other);

public:  // Destructor
    ~Vector();

public:  // Methods
    [[nodiscard]] size_t Size() const noexcept;
    [[nodiscard]] size_t Capacity() const noexcept;
    void Reserve(size_t capacity);

public:  // Operators
    const Type& operator[](size_t index) const noexcept;
    Type& operator[](size_t index) noexcept;

private:  // Fields
    RawMemory<Type> data_;
    size_t size_{0u};
};

/* RAW MEMORY IMPLEMENTATION */

template <class Type>
RawMemory<Type>::RawMemory(size_t capacity) : buffer_(Allocate(capacity)), capacity_(capacity) {}

template <class Type>
RawMemory<Type>::~RawMemory() {
    Deallocate(buffer_);
}

template <class Type>
Type* RawMemory<Type>::operator+(size_t offset) noexcept {
    assert(offset <= capacity_);
    return buffer_ + offset;
}

template <class Type>
const Type* RawMemory<Type>::operator+(size_t offset) const noexcept {
    return const_cast<RawMemory&>(*this) + offset;
}

template <class Type>
const Type& RawMemory<Type>::operator[](size_t index) const noexcept {
    return const_cast<RawMemory&>(*this)[index];
}

template <class Type>
Type& RawMemory<Type>::operator[](size_t index) noexcept {
    assert(index < capacity_);
    return buffer_[index];
}

template <class Type>
void RawMemory<Type>::Swap(RawMemory& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template <class Type>
const Type* RawMemory<Type>::GetAddress() const noexcept {
    return buffer_;
}

template <class Type>
[[maybe_unused]] Type* RawMemory<Type>::GetAddress() noexcept {
    return buffer_;
}

template <class Type>
size_t RawMemory<Type>::Capacity() const {
    return capacity_;
}

template <class Type>
Type* RawMemory<Type>::Allocate(size_t n) {
    return n != 0 ? static_cast<Type*>(operator new(n * sizeof(Type))) : nullptr;
}

template <class Type>
void RawMemory<Type>::Deallocate(Type* buf) noexcept {
    operator delete(buf);
}

/* VECTOR IMPLEMENTATION */

template <class Type>
Vector<Type>::Vector(size_t size) : data_(size), size_(size) {
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}

template <class Type>
Vector<Type>::Vector(const Vector& other) : data_(other.size_), size_(other.size_) {
    std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
}

template <class Type>
Vector<Type>::~Vector() {
    std::destroy_n(data_.GetAddress(), size_);
}

template <class Type>
size_t Vector<Type>::Size() const noexcept {
    return size_;
}

template <class Type>
size_t Vector<Type>::Capacity() const noexcept {
    return data_.Capacity();
}

template <class Type>
void Vector<Type>::Reserve(size_t capacity) {
    if (capacity > data_.Capacity()) {
        RawMemory<Type> new_data(capacity);
        if constexpr (std::is_nothrow_move_constructible_v<Type> || !std::is_copy_constructible_v<Type>) {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        } else {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }
}

template <class Type>
const Type& Vector<Type>::operator[](size_t index) const noexcept {
    return const_cast<Vector&>(*this)[index];
}

template <class Type>
Type& Vector<Type>::operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
}
