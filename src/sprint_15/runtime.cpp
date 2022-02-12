#include "runtime.h"

#include <cassert>
#include <optional>
#include <sstream>

namespace runtime {

ObjectHolder::ObjectHolder(std::shared_ptr<Object> data) : data_(std::move(data)) {}

void ObjectHolder::AssertIsValid() const {
    assert(data_ != nullptr);
}

ObjectHolder ObjectHolder::Share(Object& object) {
    return ObjectHolder(std::shared_ptr<Object>(&object, [](auto* /*p*/) { /* do nothing */ }));
}

ObjectHolder ObjectHolder::None() {
    return {};
}

Object& ObjectHolder::operator*() const {
    AssertIsValid();
    return *Get();
}

Object* ObjectHolder::operator->() const {
    AssertIsValid();
    return Get();
}

Object* ObjectHolder::Get() const {
    return data_.get();
}

ObjectHolder::operator bool() const {
    return Get() != nullptr;
}

bool IsTrue(const ObjectHolder& object) {
    if (!object)
        return false;

    if (auto* ptr = object.TryAs<Bool>())
        return ptr->GetValue();

    if (auto* ptr = object.TryAs<Number>())
        return ptr->GetValue() != 0;

    if (auto* ptr = object.TryAs<String>())
        return !ptr->GetValue().empty();

    return false;
}

void ClassInstance::Print(std::ostream& os, Context& context) {
    if (HasMethod("__str__", 0))
        Call("__str__", {}, context).Get()->Print(os, context);
    else
        os << this;
}

bool ClassInstance::HasMethod(const std::string& method_name, size_t argument_count) const {
    if (auto* method = class_.GetMethod(method_name))
        return method->formal_params.size() == argument_count;

    return false;
}

Closure& ClassInstance::Fields() {
    return fields_;
}

const Closure& ClassInstance::Fields() const {
    return fields_;
}

ClassInstance::ClassInstance(const Class& cls) : class_(cls) {}

ObjectHolder ClassInstance::Call(const std::string& method_name, const std::vector<ObjectHolder>& actual_arguments,
                                 Context& context) {
    using namespace std::string_literals;

    if (!HasMethod(method_name, actual_arguments.size()))
        throw std::runtime_error("Method "s + method_name + " does not exists"s);

    Closure arguments;
    arguments["self"s] = ObjectHolder::Share(*this);

    auto* method = class_.GetMethod(method_name);
    for (size_t argument_id = 0; argument_id < actual_arguments.size(); ++argument_id)
        arguments[method->formal_params[argument_id]] = actual_arguments[argument_id];

    return method->body->Execute(arguments, context);
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
    : name_(std::move(name)), methods_(std::move(methods)), parent_(parent) {
    // Get methods from the parent class
    if (parent_) {
        for (const auto& method : parent_->methods_)
            name_to_method_[method.name] = &method;
    }

    // Add this class methods
    for (const auto& method : methods_) {
        name_to_method_[method.name] = &method;
    }
}

const Method* Class::GetMethod(const std::string& name) const {
    if (auto position = name_to_method_.find(name); position != name_to_method_.end())
        return position->second;

    return nullptr;
}

[[nodiscard]] const std::string& Class::GetName() const {
    return name_;
}

void Class::Print(std::ostream& os, [[maybe_unused]] Context& context) {
    os << "Class " << GetName();
}

void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
    os << (GetValue() ? "True" : "False");
}

bool Equal(const ObjectHolder& left, const ObjectHolder& right, Context& context) {
    // If both ObjectHolders are empty we assume they are equal
    if (!left && !right)
        return true;

    if (left.TryAs<Bool>() && right.TryAs<Bool>())
        return left.TryAs<Bool>()->GetValue() == right.TryAs<Bool>()->GetValue();

    if (left.TryAs<Number>() && right.TryAs<Number>())
        return left.TryAs<Number>()->GetValue() == right.TryAs<Number>()->GetValue();

    if (left.TryAs<String>() && right.TryAs<String>())
        return left.TryAs<String>()->GetValue() == right.TryAs<String>()->GetValue();

    if (auto* cls = left.TryAs<ClassInstance>(); cls && cls->HasMethod("__eq__", 1))
        return cls->Call("__eq__", {right}, context).TryAs<Bool>()->GetValue();

    throw std::runtime_error("Impossible to compare two objects using operator ==");
}

bool Less(const ObjectHolder& left, const ObjectHolder& right, Context& context) {
    if (left.TryAs<Bool>() && right.TryAs<Bool>())
        return left.TryAs<Bool>()->GetValue() < right.TryAs<Bool>()->GetValue();

    if (left.TryAs<Number>() && right.TryAs<Number>())
        return left.TryAs<Number>()->GetValue() < right.TryAs<Number>()->GetValue();

    if (left.TryAs<String>() && right.TryAs<String>())
        return left.TryAs<String>()->GetValue() < right.TryAs<String>()->GetValue();

    if (auto* cls = left.TryAs<ClassInstance>(); cls && cls->HasMethod("__lt__", 1))
        return cls->Call("__lt__", {right}, context).TryAs<Bool>()->GetValue();

    throw std::runtime_error("Impossible to compare two objects using operator <");
}

bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Equal(lhs, rhs, context);
}

bool Greater(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Less(lhs, rhs, context) && !Equal(lhs, rhs, context);
}

bool LessOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Greater(lhs, rhs, context);
}

bool GreaterOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Less(lhs, rhs, context);
}

}  // namespace runtime