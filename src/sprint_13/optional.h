#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    [[nodiscard]] const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename Type>
class Optional {
public:  // Constructors & assigment operators
    Optional() = default;

    explicit Optional(const Type& value);
    explicit Optional(Type&& value);

    Optional(const Optional& other);
    Optional(Optional&& other) noexcept;

    Optional& operator=(const Type& other);
    Optional& operator=(Type&& other);

    Optional& operator=(const Optional& other);
    Optional& operator=(Optional&& other) noexcept;

public:  // Destructors
    ~Optional();

public:  // Methods
    [[nodiscard]] bool HasValue() const;

    template <typename... Args>
    void Emplace(Args&&... values);

    Type& operator*();
    const Type& operator*() const;
    Type* operator->();
    const Type* operator->() const;

    Type& Value();
    const Type& Value() const;

    void Reset();

private:
    // alignas() is necessary for correct alignment in memory
    alignas(Type) char data_[sizeof(Type)];
    bool is_initialized_{false};
    Type* value_{nullptr};
};

template <class Type>
Optional<Type>::Optional(const Type& value) {
    is_initialized_ = true;
    value_ = new (&data_[0]) Type(value);
}

template <class Type>
Optional<Type>::Optional(Type&& value) {
    is_initialized_ = true;
    value_ = new (&data_[0]) Type(std::forward<Type>(value));
}

template <class Type>
Optional<Type>::Optional(const Optional& other) {
    if (HasValue() && other.HasValue()) {
        *value_ = other.Value();
    } else if (HasValue() && !other.HasValue()) {
        is_initialized_ = false;
        value_->~Type();
    } else if (!HasValue() && other.HasValue()) {
        is_initialized_ = true;
        value_ = new (&data_[0]) Type(other.Value());
    }
}

template <class Type>
Optional<Type>::Optional(Optional&& other) noexcept {
    if (HasValue() && other.HasValue()) {
        *value_ = std::forward<Type>(other.Value());
    } else if (HasValue() && !other.HasValue()) {
        is_initialized_ = false;
        value_->~Type();
    } else if (!HasValue() && other.HasValue()) {
        is_initialized_ = true;
        value_ = new (&data_[0]) Type(std::forward<Type>(other.Value()));
    }
}

template <class Type>
Optional<Type>& Optional<Type>::operator=(const Type& other) {
    if (HasValue()) {
        *value_ = other;
    } else {
        is_initialized_ = true;
        value_ = new (&data_[0]) Type(other);
    }

    return *this;
}

template <class Type>
Optional<Type>& Optional<Type>::operator=(Type&& other) {
    if (HasValue()) {
        *value_ = std::forward<Type>(other);
    } else {
        is_initialized_ = true;
        value_ = new (&data_[0]) Type(std::forward<Type>(other));
    }

    return *this;
}

template <class Type>
Optional<Type>& Optional<Type>::operator=(const Optional& other) {
    if (this != &other) {
        if (HasValue() && other.HasValue()) {
            *value_ = other.Value();
        } else if (HasValue() && !other.HasValue()) {
            is_initialized_ = false;
            value_->~Type();
        } else if (!HasValue() && other.HasValue()) {
            is_initialized_ = true;
            value_ = new (&data_[0]) Type(other.Value());
        }
    }

    return *this;
}

template <class Type>
Optional<Type>& Optional<Type>::operator=(Optional&& other) noexcept {
    if (this != &other) {
        if (HasValue() && other.HasValue()) {
            *value_ = std::forward<Type>(other.Value());
        } else if (HasValue() && !other.HasValue()) {
            is_initialized_ = false;
            value_->~Type();
        } else if (!HasValue() && other.HasValue()) {
            is_initialized_ = true;
            value_ = new (&data_[0]) Type(std::forward<Type>(other.Value()));
        }
    }

    return *this;
}

template <class Type>
Optional<Type>::~Optional() {
    if (is_initialized_)
        Reset();
}

template <class Type>
bool Optional<Type>::HasValue() const {
    return is_initialized_;
}

template <class Type>
template <typename... Args>
void Optional<Type>::Emplace(Args&&... values) {
    if (is_initialized_)
        Reset();
    value_ = new (&data_[0]) Type(std::forward<Args>(values)...);
    is_initialized_ = true;
}

template <class Type>
Type& Optional<Type>::operator*() {
    return *value_;
}

template <class Type>
const Type& Optional<Type>::operator*() const {
    return *value_;
}

template <class Type>
Type* Optional<Type>::operator->() {
    return value_;
}

template <class Type>
const Type* Optional<Type>::operator->() const {
    return value_;
}

template <class Type>
Type& Optional<Type>::Value() {
    if (!is_initialized_)
        throw BadOptionalAccess();
    return *value_;
}

template <class Type>
const Type& Optional<Type>::Value() const {
    if (!is_initialized_)
        throw BadOptionalAccess();
    return *value_;
}

template <class Type>
void Optional<Type>::Reset() {
    is_initialized_ = false;
    value_->~Type();
}