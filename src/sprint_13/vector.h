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

    void DestroyN(size_t size);
    void CopyN(const Vector& from, Type* to);

private:  // Fields
    Type* data_{nullptr};
    size_t capacity_{0};
    size_t size_{0};
};

/* CONSTRUCTORS */

template <class Type>
Vector<Type>::Vector(size_t size) : data_(Allocate(size)), capacity_(size), size_(size) {
    for (size_t id = 0; id < size; ++id)
        new (data_ + id) Type();
}

template <class Type>
Vector<Type>::Vector(const Vector& other) : data_(Allocate(other.size_)), capacity_(other.size_), size_(other.size_) {
    CopyN(other, data_);
}

template <class Type>
Vector<Type>::~Vector() {
    DestroyN(size_);
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
    if (capacity > capacity_) {
        Type* new_data = Allocate(capacity);
        CopyN(*this, new_data);

        DestroyN(size_);
        Deallocate(data_);
        data_ = new_data;
    }

    capacity_ = capacity;
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
void Vector<Type>::DestroyN(size_t size) {
    for (size_t id = 0; id < size; ++id)
        (data_ + id)->~Type();
}

template <class Type>
void Vector<Type>::CopyValue(const Type& value, Type* to) {
    new (to) Type(value);
}

template <class Type>
void Vector<Type>::CopyN(const Vector& from, Type* to) {
    for (size_t id = 0; id < from.size_; ++id)
        new (to + id) Type(*(from.data_ + id));
}