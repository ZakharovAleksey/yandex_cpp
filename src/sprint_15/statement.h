#pragma once

#include <functional>

#include "runtime.h"

namespace ast {

using Statement = runtime::Executable;

template <typename Type>
class ValueStatement : public Statement {
public:  // Constructor
    explicit ValueStatement(Type v) : value_(std::move(v)) {}

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& /*closure*/, runtime::Context& /*context*/) override {
        return runtime::ObjectHolder::Share(value_);
    }

private:  // Fields
    Type value_;
};

using NumericConst = ValueStatement<runtime::Number>;
using StringConst = ValueStatement<runtime::String>;
using BoolConst = ValueStatement<runtime::Bool>;

class VariableValue : public Statement {
public:  // Constructors
    explicit VariableValue(const std::string& var_name);
    explicit VariableValue(std::vector<std::string> fields_sequence_);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::vector<std::string> fields_sequence_;
};

class Assignment : public Statement {
public:  // Constructors
    Assignment(std::string var, std::unique_ptr<Statement> rv);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::string var_;
    std::unique_ptr<Statement> rv_;
};

class FieldAssignment : public Statement {
public:  // Constructors
    FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    VariableValue object_;
    std::string field_name_;
    std::unique_ptr<Statement> rv_;
};

class None : public Statement {
public:  // Methods
    runtime::ObjectHolder Execute([[maybe_unused]] runtime::Closure& closure,
                                  [[maybe_unused]] runtime::Context& context) override {
        return {};
    }
};

class Print : public Statement {
public:  // Constructors
    explicit Print(std::unique_ptr<Statement> argument);
    explicit Print(std::vector<std::unique_ptr<Statement>> args);

public:  // Methods
    static std::unique_ptr<Print> Variable(const std::string& name);
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::vector<std::unique_ptr<Statement>> args_;
};

class MethodCall : public Statement {
public:  // Constructors
    MethodCall(std::unique_ptr<Statement> object, std::string method, std::vector<std::unique_ptr<Statement>> args);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::unique_ptr<Statement> object_;
    std::string method_;
    std::vector<std::unique_ptr<Statement>> args_;
};

class NewInstance : public Statement {
public:  // Constructors
    explicit NewInstance(const runtime::Class& class_);
    NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args);

public:  // Methods
    // Возвращает объект, содержащий значение типа ClassInstance
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    const runtime::Class& class__;
    std::vector<std::unique_ptr<Statement>> args_;
};

class UnaryOperation : public Statement {
public:  // Constructors
    explicit UnaryOperation(std::unique_ptr<Statement> argument) : argument_(std::move(argument)) {}

public:  // Methods
    const std::unique_ptr<Statement>& GetArg() const {
        return argument_;
    }

private:  // Fields
    std::unique_ptr<Statement> argument_;
};

class Stringify : public UnaryOperation {
public:  // Types
    using UnaryOperation::UnaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/* MATH OPERATIONS */

class BinaryOperation : public Statement {
public:  // Constructor
    BinaryOperation(std::unique_ptr<Statement> left, std::unique_ptr<Statement> right);

protected:  // Methods
    [[nodiscard]] const std::unique_ptr<Statement>& GetLeft() const;
    [[nodiscard]] const std::unique_ptr<Statement>& GetRight() const;

private:  // Fields
    std::unique_ptr<Statement> left_;
    std::unique_ptr<Statement> right_;
};

class Add : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Sub : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Mult : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Div : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/* LOGICAL OPERATIONS */

class Or : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class And : public BinaryOperation {
public:  // Types
    using BinaryOperation::BinaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Comparison : public BinaryOperation {
public:  // Types
    // clang-format off
    using Comparator = std::function<bool(const runtime::ObjectHolder&,
                                          const runtime::ObjectHolder&,
                                          runtime::Context&)>;
    // clang-format on
public:  // Constructor
    Comparison(Comparator cmp, std::unique_ptr<Statement> lhs, std::unique_ptr<Statement> rhs);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    Comparator cmp_;
};

class Not : public UnaryOperation {
public:  // Types
    using UnaryOperation::UnaryOperation;

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

/* STATEMENTS */

class Compound : public Statement {
public:  // Constructor
    template <typename... Args>
    explicit Compound(Args&&... args) {
        if constexpr (sizeof...(Args) > 0) {
            CompoundHelper(args...);
        }
    }

public:  // Methods
    void AddStatement(std::unique_ptr<Statement> statement);
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
    std::vector<std::unique_ptr<Statement>> statements_;
};

class MethodBody : public Statement {
public:  // Types
    explicit MethodBody(std::unique_ptr<Statement>&& body);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::unique_ptr<Statement> body_;
};

class Return : public Statement {
public:  // Constructor
    explicit Return(std::unique_ptr<Statement> statement) : statement_(std::move(statement)) {}

public:  // Method
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Field
    std::unique_ptr<Statement> statement_;
};

class ClassDefinition : public Statement {
public:  // Constructors
    explicit ClassDefinition(runtime::ObjectHolder cls);

public:  // Methods
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    runtime::ObjectHolder cls_;
};

class IfElse : public Statement {
public:  // Constructor
    IfElse(std::unique_ptr<Statement> condition, std::unique_ptr<Statement> if_body,
           std::unique_ptr<Statement> else_body);

public:  // Method
    runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

private:  // Fields
    std::unique_ptr<Statement> condition_;
    std::unique_ptr<Statement> if_body_;
    std::unique_ptr<Statement> else_body_;
};

}  // namespace ast