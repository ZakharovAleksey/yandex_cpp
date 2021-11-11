#pragma once

#include <cassert>
#include <cstdlib>
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
    Type* GetAddress() noexcept;
    [[nodiscard]] size_t Capacity() const;

private:  // Methods
    static Type* Allocate(size_t n);
    static void Deallocate(Type* buf) noexcept;

private:  // Fields
    Type* buffer_{nullptr};
    size_t capacity_{0};
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

private:  // Methods
    static void CopyConstruct(Type* buffer, const Type& value);
    void DestroyN(Type* buffer, size_t size);

private:  // Fields
    RawMemory<Type> data_;
    size_t size_{0};
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
Type* RawMemory<Type>::GetAddress() noexcept {
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
    size_t created_count{0};
    try {
        for (; created_count < size; ++created_count)
            new (data_ + created_count) Type();
    } catch (...) {
        DestroyN(data_.GetAddress(), created_count);
        throw;
    }
}

template <class Type>
Vector<Type>::Vector(const Vector& other) : data_(other.size_), size_(other.size_) {
    size_t created_count{0};
    try {
        for (; created_count != other.size_; ++created_count)
            CopyConstruct(data_ + created_count, other.data_[created_count]);
    } catch (...) {
        DestroyN(data_.GetAddress(), created_count);
        throw;
    }
}

template <class Type>
Vector<Type>::~Vector() {
    DestroyN(data_.GetAddress(), size_);
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
    size_t created_count{0};
    RawMemory<Type> new_data(capacity);

    try {
        if (capacity > data_.Capacity()) {
            for (; created_count != size_; ++created_count)
                CopyConstruct(new_data + created_count, data_[created_count]);

            DestroyN(data_.GetAddress(), size_);
            data_.Swap(new_data);
        }
    } catch (...) {
        DestroyN(new_data.GetAddress(), created_count);
        throw;
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

template <class Type>
void Vector<Type>::CopyConstruct(Type* buffer, const Type& value) {
    new (buffer) Type(value);
}

template <class Type>
void Vector<Type>::DestroyN(Type* buffer, size_t size) {
    for (size_t id = 0; id != size; ++id)
        (buffer + id)->~Type();
}