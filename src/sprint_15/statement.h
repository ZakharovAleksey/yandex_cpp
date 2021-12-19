#pragma once

#include <functional>

#include "runtime.h"

namespace ast {

using Statement = runtime::Executable;
using StatementUPtr = std::unique_ptr<Statement>;

template <typename Type>
class ValueStatement : public Statement {
public:  // Constructor
    explicit ValueStatement(Type value) : value_(std::move(value)) {}

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure&, runtime::Context&) override {
        return runtime::ObjectHolder::Share(value_);
    }

private:  // Fields
    Type value_;
};

using NumericConst = ValueStatement<runtime::Number>;
using StringConst = ValueStatement<runtime::String>;
using BoolConst = ValueStatement<runtime::Bool>;

/*!
 * @brief Performs chain call of class object fields
 * @example Used in expressions like: class.field0.field1.field2. ...
 */
class VariableValue : public Statement {
public:  // Constructors
    explicit VariableValue(const std::string& variable_name);
    explicit VariableValue(std::vector<std::string> fields_sequence_);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::vector<std::string> fields_sequence_;
};

/* ASSIGMENT AND NEW ITEMS CREATION */

/// @brief Assigns "right_value" value to the "variable" variable on the left
class Assignment : public Statement {
public:  // Constructors
    Assignment(std::string variable, StatementUPtr right_value);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::string variable_;
    StatementUPtr right_value_;
};

/*!
 * @brief Assign "right_value" value to the field "field_name" of object "object"
 * @example Used in cases like this: obj.field = right_value
 */
class FieldAssignment : public Statement {
public:  // Constructors
    FieldAssignment(VariableValue object, std::string field_name, StatementUPtr right_value);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    VariableValue object_;
    std::string field_name_;
    StatementUPtr right_value_;
};

/// @brief Represents None value
class None : public Statement {
public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure&, runtime::Context&) override {
        return {};
    }
};

/// @brief Represents the "print" method
class Print : public Statement {
public:  // Constructors
    explicit Print(StatementUPtr argument);
    explicit Print(std::vector<StatementUPtr> arguments);

public:  // Methods
    static std::unique_ptr<Print> Variable(const std::string& name);
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::vector<StatementUPtr> arguments_;
};

/*!
 * @brief Calls for the method "method" with arguments "arguments" from object "object"
 * @example Could be used int: object.call_method(arg_1, arg_2)
 */
class MethodCall : public Statement {
public:  // Constructors
    MethodCall(StatementUPtr object, std::string method, std::vector<StatementUPtr> arguments);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    StatementUPtr object_;
    std::string method_;
    std::vector<StatementUPtr> arguments_;
};

/*!
 * @brief Creates instance of "created_class" with arguments "arguments"
 * @example Could be used int: instance = created_class(arg_1, arg_2, arg_3)
 */
class NewInstance : public Statement {
public:  // Constructors
    explicit NewInstance(const runtime::Class& class_);
    NewInstance(const runtime::Class& class_, std::vector<StatementUPtr> arguments);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    const runtime::Class& created_class_;
    std::vector<StatementUPtr> arguments_;
};

/* UNARY OPERATIONS */

/// @brief Base class for the unary operations
class UnaryOperation : public Statement {
public:  // Constructors
    explicit UnaryOperation(StatementUPtr argument) : argument_(std::move(argument)) {}

public:  // Methods
    [[nodiscard]] const StatementUPtr& GetArg() const {
        return argument_;
    }

private:  // Fields
    StatementUPtr argument_;
};

/// @brief Returns string representation of object
class Stringify : public UnaryOperation {
public:  // Types
    using UnaryOperation::UnaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/* MATH OPERATIONS */

/// @brief Base class for the binary operations
class BinaryOperation : public Statement {
public:  // Constructor
    BinaryOperation(StatementUPtr left, StatementUPtr right);

protected:  // Methods
    [[nodiscard]] const StatementUPtr& GetLeft() const;
    [[nodiscard]] const StatementUPtr& GetRight() const;

private:  // Fields
    StatementUPtr left_;
    StatementUPtr right_;
};

/// @brief Represent plus operation
class Add : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/// @brief Represent minus operation
class Sub : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/// @brief Represent multiply operation
class Mult : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/// @brief Represent division operation
class Div : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/* LOGICAL OPERATIONS */

/// @brief Represent "logical or" operation
class Or : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/// @brief Represent "logical and" operation
class And : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/// @brief Represent "logical equal" operation
class Comparison : public BinaryOperation {
public:  // Types
    // clang-format off
    using Comparator = std::function<bool(const runtime::ObjectHolder&,
                                          const runtime::ObjectHolder&,
                                          runtime::Context&)>;
    // clang-format on
public:  // Constructor
    Comparison(Comparator comparator, StatementUPtr left, StatementUPtr right);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    Comparator comparator_;
};

/// @brief Represent "logical not" operation
class Not : public UnaryOperation {
public:  // Types
    using UnaryOperation::UnaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/* STATEMENTS */

/// @brief Base class for the composed actions, like body method, if-else
class Compound : public Statement {
public:  // Constructor
    template <typename... Args>
    explicit Compound(Args&&... args) {
        if constexpr (sizeof...(Args) > 0) {
            CompoundHelper(args...);
        }
    }

public:  // Methods
    void AddStatement(StatementUPtr statement);
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Methods
    template <typename T0, typename... Ts>
    void CompoundHelper(T0&& v0, Ts&&... vs) {
        if constexpr (sizeof...(vs) != 0) {
            statements_.push_back(std::move(v0));
            CompoundHelper(vs...);
        } else {
            statements_.push_back(std::move(v0));
        }
    }

private:  // Fields
    std::vector<StatementUPtr> statements_;
};

/// @brief Represents the method body
class MethodBody : public Statement {
public:  // Types
    explicit MethodBody(StatementUPtr&& body);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    StatementUPtr body_;
};

/// @brief Represents "return" statement
class Return : public Statement {
public:  // Constructor
    explicit Return(StatementUPtr statement) : statement_(std::move(statement)) {}

public:  // Method
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Field
    StatementUPtr statement_;
};

/// @brief Declares the class
class ClassDefinition : public Statement {
public:  // Constructors
    explicit ClassDefinition(runtime::ObjectHolder cls);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    runtime::ObjectHolder cls_;
};

/// @brief Represents if-else clause
class IfElse : public Statement {
public:  // Constructor
    IfElse(StatementUPtr condition, StatementUPtr if_body, StatementUPtr else_body);

public:  // Method
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    StatementUPtr condition_;
    StatementUPtr if_body_;
    StatementUPtr else_body_;
};

}  // namespace ast