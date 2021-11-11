#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>

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
    static Type* Allocate(size_t size);
    static void Deallocate(Type* buffer);

    static void CopyConstruct(Type* buffer, const Type& value);

    void DestroyN(Type* buffer, size_t size);

private:  // Fields
    Type* data_{nullptr};
    size_t capacity_{0};
    size_t size_{0};
};

/* CONSTRUCTORS */

template <class Type>
Vector<Type>::Vector(size_t size) : data_(Allocate(size)), capacity_(size), size_(size) {
    size_t created_count{0};
    try {
        for (; created_count < size; ++created_count)
            new (data_ + created_count) Type();
    } catch (...) {
        DestroyN(data_, created_count);
        Deallocate(data_);
        throw;
    }
}

template <class Type>
Vector<Type>::Vector(const Vector& other) : data_(Allocate(other.size_)), capacity_(other.size_), size_(other.size_) {
    size_t created_count{0};
    try {
        for (; created_count != other.size_; ++created_count)
            CopyConstruct(data_ + created_count, other.data_[created_count]);
    } catch (...) {
        DestroyN(data_, created_count);
        Deallocate(data_);
        throw;
    }
}

template <class Type>
Vector<Type>::~Vector() {
    DestroyN(data_, size_);
    Deallocate(data_);
}

/* METHODS */

template <class Type>
size_t Vector<Type>::Size() const noexcept {
    return size_;
}

template <class Type>
size_t Vector<Type>::Capacity() const noexcept {
    return capacity_;
}

template <class Type>
void Vector<Type>::Reserve(size_t capacity) {
    size_t created_count{0};
    Type* new_data{nullptr};
    try {
        if (capacity > capacity_) {
            new_data = Allocate(capacity);
            for (; created_count != size_; ++created_count) {
                CopyConstruct(new_data + created_count, data_[created_count]);
            }

            DestroyN(data_, size_);
            Deallocate(data_);

            data_ = new_data;
            capacity_ = capacity;
        }
    } catch (...) {
        DestroyN(new_data, created_count);
        Deallocate(new_data);
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
Type* Vector<Type>::Allocate(size_t size) {
    return size != 0 ? static_cast<Type*>(operator new(size * sizeof(Type))) : nullptr;
}
template <class Type>
void Vector<Type>::Deallocate(Type* buffer) {
    operator delete(buffer);
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