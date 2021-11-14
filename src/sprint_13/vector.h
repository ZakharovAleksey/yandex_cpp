#pragma once

/*
 * Description: analog of std::vector<> implementation
 */

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <new>
#include <utility>

template <typename Type>
class RawMemory {
public:  // Constructors & Assigment operators
    RawMemory() = default;
    explicit RawMemory(size_t capacity) : buffer_(Allocate(capacity)), capacity_(capacity) {}
    RawMemory(const RawMemory&) = delete;
    RawMemory(RawMemory&& other) noexcept {
        buffer_ = std::exchange(other.buffer_, nullptr);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    RawMemory& operator=(const RawMemory&) = delete;
    RawMemory& operator=(RawMemory&& other) noexcept {
        if (this != &other) {
            buffer_ = std::exchange(other.buffer_, nullptr);
            capacity_ = std::exchange(other.capacity_, 0);
        }
        return *this;
    }

public:  // Destructors
    ~RawMemory() {
        Deallocate(buffer_);
    }

public:  // Operators
    Type* operator+(size_t offset) noexcept {
        assert(offset <= capacity_);
        return buffer_ + offset;
    }
    const Type* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const Type& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }
    Type& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

public:  // Methods
    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }
    const Type* GetAddress() const noexcept {
        return buffer_;
    }
    [[maybe_unused]] Type* GetAddress() noexcept {
        return buffer_;
    }
    [[nodiscard]] size_t Capacity() const {
        return capacity_;
    }

private:  // Methods
    static Type* Allocate(size_t size) {
        return size != 0 ? static_cast<Type*>(operator new(size * sizeof(Type))) : nullptr;
    }
    static void Deallocate(Type* buffer) noexcept {
        operator delete(buffer);
    }

private:  // Fields
    Type* buffer_{nullptr};
    size_t capacity_{0};
};

template <typename Type>
class Vector {
public:  // Types
    using iterator = Type*;
    using const_iterator = const Type*;

public:  // Constructors & Assigment operators
    Vector() = default;
    explicit Vector(size_t size) : data_(size), size_(size) {
        std::uninitialized_value_construct_n(begin(), size_);
    }
    Vector(const Vector& other) : data_(other.size_), size_(other.size_) {
        std::uninitialized_copy_n(other.begin(), size_, begin());
    }
    // clang-format off
    Vector(Vector&& other) noexcept:
          data_(std::exchange(other.data_, RawMemory<Type>{})),
          size_(std::exchange(other.size_, 0)) {}
    // clang-format on

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            if (other.size_ > data_.Capacity()) {
                auto copy{other};
                Swap(copy);
            } else {
                size_t extra_count = (other.size_ < size_) ? size_ - other.size_ : other.size_ - size_;

                // In case other vector is smaller, we copy elements from him and destroy extra elements from original
                if (other.size_ < size_) {
                    for (size_t id = 0; id < other.size_; ++id)
                        data_[id] = other.data_[id];

                    std::destroy_n(begin() + other.size_, extra_count);
                } else {
                    // In case other vector is bigger, we copy elements from him and create extra elements from him
                    for (size_t id = 0; id < size_; ++id)
                        data_[id] = other.data_[id];

                    std::uninitialized_copy_n(begin() + size_, extra_count, begin() + size_);
                }
                size_ = other.size_;
            }
        }
        return *this;
    }
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            data_ = std::exchange(other.data_, RawMemory<Type>{});
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

public:  // Destructor
    ~Vector() {
        if (size_ != 0)
            std::destroy_n(begin(), size_);
    }

public:  // Methods
    iterator begin() noexcept {
        return data_.GetAddress();
    }
    iterator end() noexcept {
        return data_.GetAddress() + size_;
    }
    const_iterator begin() const noexcept {
        return data_.GetAddress();
    }
    const_iterator end() const noexcept {
        return data_.GetAddress() + size_;
    }
    const_iterator cbegin() const noexcept {
        return data_.GetAddress();
    }
    const_iterator cend() const noexcept {
        return data_.GetAddress() + size_;
    }

    template <typename... Args>
    iterator Emplace(const_iterator position, Args&&... args) {
        size_t index = std::distance(cbegin(), position);

        if (size_ == data_.Capacity()) {
            size_t count_before = std::distance(cbegin(), position);
            size_t count_after = std::distance(position, cend());

            RawMemory<Type> tmp_memory((size_ == 0) ? 1 : 2 * size_);
            new (tmp_memory.GetAddress() + index) Type(std::forward<Args>(args)...);

            try {
                // clang-format off
            if constexpr (std::is_nothrow_move_constructible_v<Type> || !std::is_copy_constructible_v<Type>) {
                std::uninitialized_move_n(begin(), count_before, tmp_memory.GetAddress());
                std::uninitialized_move_n(begin() + count_before, count_after, tmp_memory.GetAddress() + count_before + 1);
            } else {
                std::uninitialized_copy_n(begin(), count_before, tmp_memory.GetAddress());
                std::uninitialized_copy_n(begin() + count_before, count_after, tmp_memory.GetAddress() + count_before + 1);
            }
                // clang-format on
            } catch (...) {
                tmp_memory.~RawMemory();
            }

            std::destroy_n(begin(), size_);
            data_.Swap(tmp_memory);
        } else {
            // In case we insert not in empty vector or not in the end of the vector, we need to shift elements
            if (size_ != index) {
                new (end()) Type(std::forward<Type>(*(end() - 1)));
                std::move_backward(begin() + index, end() - 1, end());
                data_[index] = Type(std::forward<Args>(args)...);
            } else {
                new (begin() + index) Type(std::forward<Args>(args)...);
            }
        }

        ++size_;
        return begin() + index;
    }

    iterator Erase(const_iterator position) noexcept(std::is_nothrow_move_assignable_v<Type>) {
        size_t index = std::distance(cbegin(), position);

        std::move(begin() + index + 1, end(), begin() + index);
        std::destroy_n(end() - 1, 1);

        --size_;
        return begin() + index;
    }
    iterator Insert(const_iterator position, const Type& value) {
        return Emplace(position, value);
    }
    iterator Insert(const_iterator position, Type&& value) {
        return Emplace(position, std::move(value));
    }

    template <typename... Args>
    Type& EmplaceBack(Args&&... args) {
        if (size_ == data_.Capacity()) {
            RawMemory<Type> tmp_memory((size_ == 0) ? 1 : 2 * size_);

            new (tmp_memory.GetAddress() + size_) Type(std::forward<Args>(args)...);
            if constexpr (std::is_nothrow_move_constructible_v<Type> || !std::is_copy_constructible_v<Type>) {
                std::uninitialized_move_n(begin(), size_, tmp_memory.GetAddress());
            } else {
                std::uninitialized_copy_n(begin(), size_, tmp_memory.GetAddress());
            }

            std::destroy_n(begin(), size_);
            data_.Swap(tmp_memory);
        } else {
            new (begin() + size_) Type(std::forward<Args>(args)...);
        }

        ++size_;
        return *(begin() + size_ - 1);
    }

    [[nodiscard]] size_t Size() const noexcept {
        return size_;
    }
    [[nodiscard]] size_t Capacity() const noexcept {
        return data_.Capacity();
    }
    void Reserve(size_t capacity) {
        if (capacity > data_.Capacity()) {
            RawMemory<Type> tmp_memory(capacity);
            if constexpr (std::is_nothrow_move_constructible_v<Type> || !std::is_copy_constructible_v<Type>) {
                std::uninitialized_move_n(begin(), size_, tmp_memory.GetAddress());
            } else {
                std::uninitialized_copy_n(begin(), size_, tmp_memory.GetAddress());
            }
            std::destroy_n(begin(), size_);
            data_.Swap(tmp_memory);
        }
    }
    void Swap(Vector<Type>& other) {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }
    void Resize(size_t new_size) {
        if (new_size > data_.Capacity()) {
            Reserve(new_size);
            std::uninitialized_value_construct_n(begin(), new_size - size_);
        } else {
            std::destroy_n(begin() + new_size, size_ - new_size);
        }
        size_ = new_size;
    }
    void PushBack(const Type& value) {
        if (size_ == data_.Capacity()) {
            // Step 1. Allocate new memory
            RawMemory<Type> tmp_memory((size_ == 0) ? 1 : 2 * size_);

            // Step 2. !!! Move existed value to the end of the memory (value could be vector's element) !!!
            new (tmp_memory.GetAddress() + size_) Type(value);

            // Step 3. Copy the rest elements to the new memory
            if constexpr (std::is_nothrow_move_constructible_v<Type> || !std::is_copy_constructible_v<Type>) {
                std::uninitialized_move_n(begin(), size_, tmp_memory.GetAddress());
            } else {
                std::uninitialized_copy_n(begin(), size_, tmp_memory.GetAddress());
            }
            std::destroy_n(begin(), size_);

            // Step 4. Swap memories
            data_.Swap(tmp_memory);
        } else {
            new (begin() + size_) Type(value);
        }
        ++size_;
    }
    void PushBack(Type&& value) {
        if (size_ == data_.Capacity()) {
            RawMemory<Type> tmp_memory((size_ == 0) ? 1 : 2 * size_);

            new (tmp_memory.GetAddress() + size_) Type(std::move(value));
            if constexpr (std::is_nothrow_move_constructible_v<Type> || !std::is_copy_constructible_v<Type>) {
                std::uninitialized_move_n(begin(), size_, tmp_memory.GetAddress());
            } else {
                std::uninitialized_copy_n(begin(), size_, tmp_memory.GetAddress());
            }

            std::destroy_n(begin(), size_);
            data_.Swap(tmp_memory);
        } else {
            new (begin() + size_) Type(std::move(value));
        }
        ++size_;
    }
    void PopBack() noexcept {
        std::destroy_n(begin() + size_ - 1, 1);
        --size_;
    }

public:  // Operators
    const Type& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

private:  // Fields
    RawMemory<Type> data_;
    size_t size_{0u};
};
