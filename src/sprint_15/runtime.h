#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace runtime {

class Context {
public:  // Methods
    virtual std::ostream& GetOutputStream() = 0;

protected:  // Destructor
    ~Context() = default;
};

/// @brief Base class for all Mython objects
class Object {
public:  // Destructor
    virtual ~Object() = default;

public:  // Methods
    virtual void Print(std::ostream& os, Context& context) = 0;
};

class ObjectHolder {
public:  // Constructor
    ObjectHolder() = default;

public:  // Methods
    /*!
     * @brief Returns an ObjectHolder that owns an object of type Type
     * @tparam Type Type of object
     * @return Holder for the object
     */
    template <typename Type>
    [[nodiscard]] static ObjectHolder Own(Type&& object) {
        return ObjectHolder(std::make_shared<Type>(std::forward<Type>(object)));
    }

    /// @brief Creates an ObjectHolder that does not own the object (analogous to a weak reference)
    [[nodiscard]] static ObjectHolder Share(Object& object);
    /// @brief Creates an empty ObjectHolder corresponding to None
    [[nodiscard]] static ObjectHolder None();

    /*!
     *  @brief Returns a reference to the Object inside the ObjectHolder.
     *  @attention ObjectHolder must be non-empty
     */
    Object& operator*() const;

    Object* operator->() const;

    [[nodiscard]] Object* Get() const;

    /*!
     *  @brief Try to get a pointer to the object of specified type
     *  @tparam T Type of object
     *  @return Pointer to the object if it is in ObjectHolder, nullptr otherwise
     */
    template <typename T>
    [[nodiscard]] T* TryAs() const {
        return dynamic_cast<T*>(this->Get());
    }

public:  // Operators
    /// @brief Returns true if the ObjectHolder is not empty
    explicit operator bool() const;

private:  // Constructor
    explicit ObjectHolder(std::shared_ptr<Object> data);

private:  // Methods
    void AssertIsValid() const;

private:  // Fields
    std::shared_ptr<Object> data_{nullptr};
};

template <typename Type>
class ValueObject : public Object {
public:                      // Constructor
    ValueObject(Type value)  // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        : value_(value) {}

public:  // Methods
    void Print(std::ostream& os, [[maybe_unused]] Context& context) override {
        os << value_;
    }

    [[nodiscard]] const Type& GetValue() const {
        return value_;
    }

private:  // Fields
    Type value_{};
};

using Closure = std::unordered_map<std::string, ObjectHolder>;

bool IsTrue(const ObjectHolder& object);

/// @brief An interface for performing actions on Mython objects
class Executable {
public:  // Destructor
    virtual ~Executable() = default;

public:  // Methods
    /*!
     * @brief Performs an action on objects inside a closure using context
     * @return Returns the resulting value or None
     */
    virtual ObjectHolder Execute(Closure& closure, Context& context) = 0;
};

/* MYTHON TYPES */

using String = ValueObject<std::string>;
using Number = ValueObject<int>;

class Bool : public ValueObject<bool> {
public:  // Types
    using ValueObject<bool>::ValueObject;

public:  // Methods
    void Print(std::ostream& os, Context& context) override;
};

struct Method {
    /// @brief Name of the method
    std::string name;
    /// @brief Formal parameters names
    std::vector<std::string> formal_params;
    /// @brief Methods body
    std::unique_ptr<Executable> body;
};

class Class : public Object {
public:
    /*!
     *  @brief Создаёт класс с именем name и набором методов methods, унаследованный от класса parent.
     *  Если parent равен nullptr, то создаётся базовый класс
     */
    explicit Class(std::string name, std::vector<Method> methods, const Class* parent);

    /// @brief Returns a pointer to the method name or nullptr if there is no method with this name
    [[nodiscard]] const Method* GetMethod(const std::string& name) const;

    /// @brief Returns the name fo the class
    [[nodiscard]] const std::string& GetName() const;

    /*!
     *  @brief Prints the string "Class <class name>" to ostream
     *  @example For the class Cat it will be: "Class cat"
     */
    void Print(std::ostream& os, Context& context) override;

private:  // Fields
    std::string name_;
    std::vector<Method> methods_;
    const Class* parent_;
    std::unordered_map<std::string_view, const Method*> name_to_method_;
};

class ClassInstance : public Object {
public:  // Constructor
    explicit ClassInstance(const Class& cls);

public:  // Methods
    /// @brief Calls for the __str__ method, if exists, otherwise prints the object address
    void Print(std::ostream& os, Context& context) override;

    /*!
     *  @brief Calls for the specified method with arguments
     *  @param method_name Name of the method
     *  @param actual_arguments Method's arguments
     *  @param context Method execution context
     *  @return Method return value
     *  @throws std::runtime_error in case object or his parents don't have this method
     */
    ObjectHolder Call(const std::string& method_name, const std::vector<ObjectHolder>& actual_arguments,
                      Context& context);

    [[nodiscard]] bool HasMethod(const std::string& method_name, size_t argument_count) const;

    [[nodiscard]] Closure& Fields();

    [[nodiscard]] const Closure& Fields() const;

private:  // Fields
    const Class& class_;
    Closure fields_;
};

bool Equal(const ObjectHolder& left, const ObjectHolder& right, Context& context);

bool Less(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context);

bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context);

bool Greater(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context);

bool LessOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context);

bool GreaterOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context);

struct DummyContext : Context {
    std::ostream& GetOutputStream() override {
        return output;
    }

    std::ostringstream output;
};

class SimpleContext : public runtime::Context {
public:  // Constructor
    explicit SimpleContext(std::ostream& output) : output_(output) {}

public:  // Methods
    std::ostream& GetOutputStream() override {
        return output_;
    }

private:  // Fields
    std::ostream& output_;
};

}  // namespace runtime