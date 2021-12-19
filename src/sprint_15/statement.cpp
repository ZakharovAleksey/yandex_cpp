#include "statement.h"

#include <iostream>
#include <optional>
#include <stdexcept>

using namespace std;

namespace ast {

using runtime::Closure;
using runtime::Context;
using runtime::ObjectHolder;

namespace {
const string kAddMethod = "__add__"s;
const string kInitMethod = "__init__"s;

enum class Operation { Plus, Minus, Multiply, Divide };

Closure& TryGetFields(const ObjectHolder& object) {
    if (auto* cls = object.TryAs<runtime::ClassInstance>())
        return cls->Fields();
    throw std::runtime_error("Attempt to access non-existed field of class");
}

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

/* ASSIGMENT AND NEW ITEMS CREATION */

VariableValue::VariableValue(const std::string& variable_name) {
    fields_sequence_.push_back(variable_name);
}

VariableValue::VariableValue(std::vector<std::string> fields_sequence_)
    : fields_sequence_(std::move(fields_sequence_)) {}

ObjectHolder VariableValue::Execute(Closure& closure, Context&) {
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

Assignment::Assignment(std::string variable, StatementUPtr right_value)
    : variable_(std::move(variable)), right_value_(std::move(right_value)) {}

ObjectHolder Assignment::Execute(Closure& closure, Context& context) {
    closure[variable_] = std::move(right_value_->Execute(closure, context));
    return closure[variable_];
}

FieldAssignment::FieldAssignment(VariableValue object, std::string field_name, StatementUPtr right_value)
    : object_(std::move(object)), field_name_(std::move(field_name)), right_value_(std::move(right_value)) {}

ObjectHolder FieldAssignment::Execute(Closure& closure, Context& context) {
    auto& fields = TryGetFields(object_.Execute(closure, context));
    fields[field_name_] = right_value_->Execute(closure, context);

    return fields[field_name_];
}

Print::Print(unique_ptr<Statement> argument) {
    arguments_.push_back(std::move(argument));
}

Print::Print(vector<unique_ptr<Statement>> arguments) : arguments_(std::move(arguments)) {}

unique_ptr<Print> Print::Variable(const std::string& name) {
    return std::make_unique<Print>(std::make_unique<VariableValue>(name));
}

ObjectHolder Print::Execute(Closure& closure, Context& context) {
    auto& output = context.GetOutputStream();
    bool is_first = true;

    for (const auto& arg : arguments_) {
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

MethodCall::MethodCall(StatementUPtr object, std::string method, std::vector<StatementUPtr> arguments)
    : object_(std::move(object)), method_(std::move(method)), arguments_(std::move(arguments)) {}

ObjectHolder MethodCall::Execute(Closure& closure, Context& context) {
    std::vector<runtime::ObjectHolder> arguments;
    arguments.reserve(arguments_.size());

    for (const auto& arg : arguments_)
        arguments.emplace_back(arg->Execute(closure, context));

    if (auto* cls = object_->Execute(closure, context).TryAs<runtime::ClassInstance>()) {
        return cls->Call(method_, arguments, context);
    } else {
        throw std::runtime_error("Attempt to execute non-existed method");
    }
}

NewInstance::NewInstance(const runtime::Class& class_, std::vector<StatementUPtr> arguments)
    : created_class_(class_), arguments_(std::move(arguments)) {}

NewInstance::NewInstance(const runtime::Class& class_) : created_class_(class_) {}

ObjectHolder NewInstance::Execute(Closure& closure, Context& context) {
    auto object = ObjectHolder::Own(runtime::ClassInstance(created_class_));
    auto* object_class = object.TryAs<runtime::ClassInstance>();

    if (object_class->HasMethod(kInitMethod, arguments_.size())) {
        std::vector<runtime::ObjectHolder> arguments;
        arguments.reserve(arguments_.size());

        for (const auto& arg : arguments_)
            arguments.emplace_back(arg->Execute(closure, context));

        object_class->Call(kInitMethod, arguments, context);
    }

    return object;
}

/* UNARY OPERATIONS */

ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
    auto object = GetArg()->Execute(closure, context);
    std::stringstream output;

    if (auto* object_number = object.TryAs<runtime::Number>()) {
        object_number->Print(output, context);
        return ObjectHolder::Own(runtime::String(output.str()));
    }

    if (auto* object_string = object.TryAs<runtime::String>()) {
        object_string->Print(output, context);
        return ObjectHolder::Own(runtime::String(output.str()));
    }

    if (auto* object_bool = object.TryAs<runtime::Bool>()) {
        object_bool->Print(output, context);
        return ObjectHolder::Own(runtime::String(output.str()));
    }

    if (auto* object_class = object.TryAs<runtime::ClassInstance>()) {
        object_class->Print(output, context);
        return ObjectHolder::Own(runtime::String(output.str()));
    }

    return ObjectHolder::Own(runtime::String("None"));
}

/* BINARY OPERATIONS */

BinaryOperation::BinaryOperation(StatementUPtr left, StatementUPtr right)
    : left_(std::move(left)), right_(std::move(right)) {}

[[nodiscard]] const StatementUPtr& BinaryOperation::GetLeft() const {
    return left_;
}

[[nodiscard]] const StatementUPtr& BinaryOperation::GetRight() const {
    return right_;
}

/* MATH OPERATIONS */

ObjectHolder Add::Execute(Closure& closure, Context& context) {
    if (!GetRight() || !GetLeft())
        throw std::runtime_error("Attempt to use operator + with at least one null value");

    auto left_object = GetLeft()->Execute(closure, context);
    auto right_object = GetRight()->Execute(closure, context);

    if (auto result = NumberHelper(Operation::Plus, left_object, right_object); result.has_value())
        return result.value();

    // clang-format off
    auto result = [](const ObjectHolder& left_holder, const ObjectHolder& right_holder) {
        auto* left = left_holder.TryAs<runtime::String>();
        auto* right = right_holder.TryAs<runtime::String>();
        return (left && right) ? std::make_optional(ObjectHolder::Own(runtime::String{left->GetValue() + right->GetValue()}))
                               : std::nullopt;
    }(left_object, right_object);
    // clang-format on

    if (result.has_value())
        return result.value();

    if (auto* left = left_object.TryAs<runtime::ClassInstance>(); left && left->HasMethod(kAddMethod, 1))
        return left->Call(kAddMethod, {right_object}, context);

    throw std::runtime_error("Attempt to use operator + with unsupported operand types");
}

ObjectHolder Sub::Execute(Closure& closure, Context& context) {
    if (!GetRight() || !GetLeft())
        throw std::runtime_error("Attempt to use operator - with at least one null value");

    auto left_object = GetLeft()->Execute(closure, context);
    auto right_object = GetRight()->Execute(closure, context);

    if (auto result = NumberHelper(Operation::Minus, left_object, right_object); result.has_value())
        return result.value();

    throw std::runtime_error("Attempt to use operator - with unsupported operand types");
}

ObjectHolder Mult::Execute(Closure& closure, Context& context) {
    if (!GetRight() || !GetLeft())
        throw std::runtime_error("Attempt to use operator * with at least one null value");

    auto left_object = GetLeft()->Execute(closure, context);
    auto right_object = GetRight()->Execute(closure, context);

    if (auto result = NumberHelper(Operation::Multiply, left_object, right_object); result.has_value())
        return result.value();

    throw std::runtime_error("Attempt to use operator * with unsupported operand types");
}

ObjectHolder Div::Execute(Closure& closure, Context& context) {
    if (!GetRight() || !GetLeft())
        throw std::runtime_error("Attempt to use operator / with at least one null value");

    auto left_object = GetLeft()->Execute(closure, context);
    auto right_object = GetRight()->Execute(closure, context);

    if (auto result = NumberHelper(Operation::Divide, left_object, right_object); result.has_value())
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

Comparison::Comparison(Comparator comparator, unique_ptr<Statement> left, unique_ptr<Statement> right)
    : BinaryOperation(std::move(left), std::move(right)), comparator_(std::move(comparator)) {}

ObjectHolder Comparison::Execute(Closure& closure, Context& context) {
    auto left = GetLeft()->Execute(closure, context);
    auto right = GetRight()->Execute(closure, context);

    return ObjectHolder::Own(runtime::Bool(comparator_(left, right, context)));
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

void Compound::AddStatement(StatementUPtr statement) {
    statements_.push_back(std::move(statement));
}

ObjectHolder Compound::Execute(Closure& closure, Context& context) {
    for (const auto& statement : statements_)
        statement->Execute(closure, context);

    return {};
}

MethodBody::MethodBody(StatementUPtr&& body) : body_(std::move(body)) {}

ObjectHolder MethodBody::Execute(Closure& closure, Context& context) {
    ObjectHolder result = ObjectHolder::None();

    try {
        result = std::move(body_->Execute(closure, context));
    } catch (ObjectHolder& obj) {
        // Return statement
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

IfElse::IfElse(StatementUPtr condition, StatementUPtr if_body, StatementUPtr else_body)
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