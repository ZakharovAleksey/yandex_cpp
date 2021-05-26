//
// Created by azakharov on 5/25/2021.
//

#pragma once

template <typename Type>
class ArrayPtr {
public:  // Constructors and assigment operators
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        raw_ptr_ = size == 0 ? nullptr : new Type[size];
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept : raw_ptr_(raw_ptr) {}

    ArrayPtr(const ArrayPtr&) = delete;

    ArrayPtr& operator=(const ArrayPtr&) = delete;

public:  // Destructor
    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

public: // Methods
    [[nodiscard]] Type* Release() noexcept {
        Type* tmp = raw_ptr_;
        raw_ptr_ = nullptr;
        return tmp;
    }

    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }

    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    explicit operator bool() const {
        return raw_ptr_;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:  // Fields
    Type* raw_ptr_{nullptr};
};
