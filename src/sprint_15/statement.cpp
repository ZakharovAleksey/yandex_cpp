#include "statement.h"

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace ast {

using runtime::Closure;
using runtime::Context;
using runtime::ObjectHolder;

namespace {
static const string kAddMethod = "__add__"s;
static const string kInitMethod = "__init__"s;

enum class Operation { Plus, Minus, Multiply, Divide };

Closure& TryGetFields(const ObjectHolder& object) {
    if (auto* cls = object.TryAs<runtime::ClassInstance>())
        return cls->Fields();
    throw std::runtime_error("Attempt to access non-existed field of class");
};

std::optional<ObjectHolder> NumberHelper(Operation type, const ObjectHolder& left_holder,
                                         const ObjectHolder& right_holder) {
    auto* left = left_holder.TryAs<runtime::Number>();
    auto* right = right_holder.TryAs<runtime::Number>();

    if (!left || !right)
        return std::nullopt;

    runtime::Number result{0};
    switch (type) {
        case Operation::Plus:
            result = left->GetValue() + right->GetValue();
            break;
        case Operation::Minus:
            result = left->GetValue() - right->GetValue();
            break;
        case Operation::Multiply:
            result = left->GetValue() * right->GetValue();
            break;
        case Operation::Divide: {
            if (right->GetValue() == 0)
                throw std::runtime_error("Division by zero");
            result = left->GetValue() / right->GetValue();
            break;
        }
    }

    return std::make_optional(ObjectHolder::Own(runtime::Number{result}));
}

}  // namespace

VariableValue::VariableValue(const std::string& var_name) {
    fields_sequence_.push_back(var_name);
}

VariableValue::VariableValue(std::vector<std::string> fields_sequence_)
    : fields_sequence_(std::move(fields_sequence_)) {}

ObjectHolder VariableValue::Execute(Closure& closure, Context& /*context*/) {
    auto try_get_first = [&](Closure& object_closure, const std::string& name) {
        if (auto position = object_closure.find(name); position != object_closure.end())
            return position->second;
        throw std::runtime_error("Attempt to call a non-existed method of class");
    };

    // In case of object call method
    if (fields_sequence_.size() == 1)
        return try_get_first(closure, fields_sequence_.front());

    // Loop over all sequence of objects: obj1.obj2.obj3 ...
    auto current_object = closure.at(fields_sequence_.front());
    auto& fields = TryGetFields(current_object);
    for (size_t id = 1; id < fields_sequence_.size() - 1; ++id) {
        if (auto position = fields.find(fields_sequence_[id]); position != fields.end()) {
            current_object = position->second;
        } else {
            throw std::runtime_error("Attempt to access non-existed object");
        }
    }

    fields = current_object.TryAs<runtime::ClassInstance>()->Fields();
    return try_get_first(fields, fields_sequence_.back());
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) : var_(std::move(var)), rv_(std::move(rv)) {}

ObjectHolder Assignment::Execute(Closure& closure, Context& context) {
    closure[var_] = std::move(rv_->Execute(closure, context));
    return closure[var_];
}

FieldAssignment::FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv)
    : object_(std::move(object)), field_name_(std::move(field_name)), rv_(std::move(rv)) {}

ObjectHolder FieldAssignment::Execute(Closure& closure, Context& context) {
    auto& fields = TryGetFields(object_.Execute(closure, context));
    fields[field_name_] = rv_->Execute(closure, context);

    return fields[field_name_];
}

Print::Print(unique_ptr<Statement> argument) {
    args_.push_back(std::move(argument));
}

Print::Print(vector<unique_ptr<Statement>> args) : args_(std::move(args)) {}

unique_ptr<Print> Print::Variable(const std::string& name) {
    return std::make_unique<Print>(std::make_unique<VariableValue>(name));
}

// TODO: mb here
ObjectHolder Print::Execute(Closure& closure, Context& context) {
    auto& output = context.GetOutputStream();
    bool is_first = true;

    for (const auto& arg : args_) {
        if (!is_first)
            output << ' ';

        if (auto value = arg->Execute(closure, context)) {
            value->Print(context.GetOutputStream(), context);
        } else {
            output << "None";
        }

        is_first = false;
    }
    output << "\n";
    return ObjectHolder::None();
}

MethodCall::MethodCall(std::unique_ptr<Statement> object, std::string method,
                       std::vector<std::unique_ptr<Statement>> args)
    : object_(std::move(object)), method_(std::move(method)), args_(std::move(args)) {}

ObjectHolder MethodCall::Execute(Closure& closure, Context& context) {
    std::vector<runtime::ObjectHolder> arguments;
    arguments.reserve(args_.size());

    for (const auto& arg : args_)
        arguments.emplace_back(arg->Execute(closure, context));

    if (auto* cls = object_->Execute(closure, context).TryAs<runtime::ClassInstance>()) {
        return cls->Call(method_, arguments, context);
    } else {
        throw std::runtime_error("Attempt to execute non-existed method");
    }
}

NewInstance::NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args)
    : class__(class_), args_(std::move(args)) {}

NewInstance::NewInstance(const runtime::Class& class_) : class__(class_) {}

ObjectHolder NewInstance::Execute(Closure& closure, Context& context) {
    auto holder = ObjectHolder::Own(runtime::ClassInstance(class__));
    auto* object = holder.TryAs<runtime::ClassInstance>();

    if (object->HasMethod(kInitMethod, args_.size())) {
        std::vector<runtime::ObjectHolder> arguments;
        arguments.reserve(args_.size());

        for (const auto& arg : args_)
            arguments.emplace_back(arg->Execute(closure, context));

        object->Call(kInitMethod, arguments, context);
    }

    return holder;
}

ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
    auto holder = GetArg()->Execute(closure, context);
    std::stringstream output;

    if (auto* ptr = holder.TryAs<runtime::Number>()) {
        ptr->Print(output, context);
        return ObjectHolder::Own(runtime::String(output.str()));
    }

    if (auto* ptr = holder.TryAs<runtime::String>()) {
        ptr->Print(output, context);
        return ObjectHolder::Own(runtime::String(output.str()));
    }

    if (auto* ptr = holder.TryAs<runtime::Bool>()) {
        ptr->Print(output, context);
        return ObjectHolder::Own(runtime::String(output.str()));
    }

    if (auto* ptr = holder.TryAs<runtime::ClassInstance>()) {
        ptr->Print(output, context);
        return ObjectHolder::Own(runtime::String(output.str()));
    }

    return ObjectHolder::Own(runtime::String("None"));
}

/* BINARY OPERATIONS */

BinaryOperation::BinaryOperation(std::unique_ptr<Statement> left, std::unique_ptr<Statement> right)
    : left_(std::move(left)), right_(std::move(right)) {}

[[nodiscard]] const std::unique_ptr<Statement>& BinaryOperation::GetLeft() const {
    return left_;
}

[[nodiscard]] const std::unique_ptr<Statement>& BinaryOperation::GetRight() const {
    return right_;
}

/* BINARY MATH OPERATIONS */

ObjectHolder Add::Execute(Closure& closure, Context& context) {
    if (!GetRight() || !GetLeft())
        throw std::runtime_error("Attempt to use operator + with at least one null value");

    auto left_holder = GetLeft()->Execute(closure, context);
    auto right_holder = GetRight()->Execute(closure, context);

    if (auto result = NumberHelper(Operation::Plus, left_holder, right_holder); result.has_value())
        return result.value();

    // clang-format off
    auto result = [](const ObjectHolder& left_holder, const ObjectHolder& right_holder) {
        auto* left = left_holder.TryAs<runtime::String>();
        auto* right = right_holder.TryAs<runtime::String>();
        return (left && right) ? std::make_optional(ObjectHolder::Own(runtime::String{left->GetValue() + right->GetValue()}))
                               : std::nullopt;
    }(left_holder, right_holder);
    // clang-format on

    if (result.has_value())
        return result.value();

    if (auto* left = left_holder.TryAs<runtime::ClassInstance>(); left && left->HasMethod(kAddMethod, 1))
        return left->Call(kAddMethod, {right_holder}, context);

    throw std::runtime_error("Attempt to use operator + with unsupported operand types");
}

ObjectHolder Sub::Execute(Closure& closure, Context& context) {
    if (!GetRight() || !GetLeft())
        throw std::runtime_error("Attempt to use operator - with at least one null value");

    auto left_holder = GetLeft()->Execute(closure, context);
    auto right_holder = GetRight()->Execute(closure, context);

    if (auto result = NumberHelper(Operation::Minus, left_holder, right_holder); result.has_value())
        return result.value();

    throw std::runtime_error("Attempt to use operator - with unsupported operand types");
}

ObjectHolder Mult::Execute(Closure& closure, Context& context) {
    if (!GetRight() || !GetLeft())
        throw std::runtime_error("Attempt to use operator * with at least one null value");

    auto left_holder = GetLeft()->Execute(closure, context);
    auto right_holder = GetRight()->Execute(closure, context);

    if (auto result = NumberHelper(Operation::Multiply, left_holder, right_holder); result.has_value())
        return result.value();

    throw std::runtime_error("Attempt to use operator * with unsupported operand types");
}

ObjectHolder Div::Execute(Closure& closure, Context& context) {
    if (!GetRight() || !GetLeft())
        throw std::runtime_error("Attempt to use operator / with at least one null value");

    auto left_holder = GetLeft()->Execute(closure, context);
    auto right_holder = GetRight()->Execute(closure, context);

    if (auto result = NumberHelper(Operation::Divide, left_holder, right_holder); result.has_value())
        return result.value();

    throw std::runtime_error("Attempt to use operator / with unsupported operand types");
}

/* LOGICAL OPERATIONS */

ObjectHolder Or::Execute(Closure& closure, Context& context) {
    // clang-format off
    auto execute = [&](auto* object) {
        return object->GetValue() ? GetLeft()->Execute(closure, context) :
                                    GetRight()->Execute(closure, context);
    };
    // clang-format on

    auto object = GetLeft()->Execute(closure, context);

    if (auto* object_bool = object.TryAs<runtime::Bool>())
        return execute(object_bool);

    if (auto* object_number = object.TryAs<runtime::Number>())
        return execute(object_number);

    throw std::runtime_error("Attempt to use operator OR with incompatible types");
}

ObjectHolder And::Execute(Closure& closure, Context& context) {
    // clang-format off
    auto execute = [&](auto* object) {
        return !object->GetValue() ? GetLeft()->Execute(closure, context) :
                                     GetRight()->Execute(closure, context);
    };
    // clang-format on

    auto object = GetLeft()->Execute(closure, context);

    if (auto* object_bool = object.TryAs<runtime::Bool>())
        return execute(object_bool);

    if (auto* object_number = object.TryAs<runtime::Number>())
        return execute(object_number);

    throw std::runtime_error("Attempt to use operator AND with incompatible types");
}

Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
    : BinaryOperation(std::move(lhs), std::move(rhs)), cmp_(std::move(cmp)) {}

ObjectHolder Comparison::Execute(Closure& closure, Context& context) {
    auto left = GetLeft()->Execute(closure, context);
    auto right = GetRight()->Execute(closure, context);

    return ObjectHolder::Own(runtime::Bool(cmp_(left, right, context)));
}

ObjectHolder Not::Execute(Closure& closure, Context& context) {
    auto object = GetArg()->Execute(closure, context);

    if (auto object_bool = object.TryAs<runtime::Bool>())
        return ObjectHolder::Own(runtime::Bool{!object_bool->GetValue()});

    if (auto object_number = object.TryAs<runtime::Number>())
        return ObjectHolder::Own(runtime::Bool{!object_number->GetValue()});

    throw std::runtime_error("Attempt to use operator NOT with incompatible type");
}

/* STATEMENTS */

void Compound::AddStatement(std::unique_ptr<Statement> statement) {
    statements_.push_back(std::move(statement));
}

ObjectHolder Compound::Execute(Closure& closure, Context& context) {
    for (const auto& statement : statements_)
        statement->Execute(closure, context);

    return {};
}

MethodBody::MethodBody(std::unique_ptr<Statement>&& body) : body_(std::move(body)) {}

ObjectHolder MethodBody::Execute(Closure& closure, Context& context) {
    ObjectHolder result = ObjectHolder::None();

    try {
        result = std::move(body_->Execute(closure, context));
    } catch (ObjectHolder& obj) {
        // Use Return statement
        result = std::move(obj);
    }
    return result;
}

ObjectHolder Return::Execute(Closure& closure, Context& context) {
    throw statement_->Execute(closure, context);
}

ClassDefinition::ClassDefinition(ObjectHolder cls) : cls_(std::move(cls)) {}

ObjectHolder ClassDefinition::Execute(Closure& closure, Context& /*context*/) {
    closure[cls_.TryAs<runtime::Class>()->GetName()] = cls_;
    return cls_;
}

IfElse::IfElse(std::unique_ptr<Statement> condition, std::unique_ptr<Statement> if_body,
               std::unique_ptr<Statement> else_body)
    : condition_(std::move(condition)), if_body_(std::move(if_body)), else_body_(std::move(else_body)) {}

ObjectHolder IfElse::Execute(Closure& closure, Context& context) {
    auto condition = condition_->Execute(closure, context);

    if (runtime::IsTrue(condition)) {
        return if_body_->Execute(closure, context);
    } else if (else_body_) {
        return else_body_->Execute(closure, context);
    } else {
        return {};
    }
}

}  // namespace ast