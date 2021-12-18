#include "runtime.h"

#include <cassert>
#include <optional>
#include <sstream>

using namespace std;

namespace runtime {

ObjectHolder::ObjectHolder(std::shared_ptr<Object> data) : data_(std::move(data)) {}

void ObjectHolder::AssertIsValid() const {
    assert(data_ != nullptr);
}

ObjectHolder ObjectHolder::Share(Object& object) {
    // Возвращаем невладеющий shared_ptr (его deleter ничего не делает)
    return ObjectHolder(std::shared_ptr<Object>(&object, [](auto* /*p*/) { /* do nothing */ }));
}

ObjectHolder ObjectHolder::None() {
    return ObjectHolder();
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

bool IsTrue(const ObjectHolder& /*object*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return false;
}

void ClassInstance::Print(std::ostream& /*os*/, Context& /*context*/) {
    // Заглушка, реализуйте метод самостоятельно
}

bool ClassInstance::HasMethod(const std::string& /*method*/, size_t /*argument_count*/) const {
    // Заглушка, реализуйте метод самостоятельно
    return false;
}

Closure& ClassInstance::Fields() {
    // Заглушка. Реализуйте метод самостоятельно
    throw std::logic_error("Not implemented"s);
}

const Closure& ClassInstance::Fields() const {
    // Заглушка. Реализуйте метод самостоятельно
    throw std::logic_error("Not implemented"s);
}

ClassInstance::ClassInstance(const Class& /*cls*/) {
    // Реализуйте метод самостоятельно
}

ObjectHolder ClassInstance::Call(const std::string& /*method*/, const std::vector<ObjectHolder>& /*actual_args*/,
                                 Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно.
    throw std::runtime_error("Not implemented"s);
}

Class::Class(std::string /*name*/, std::vector<Method> /*methods*/, const Class* /*parent*/) {
    // Реализуйте метод самостоятельно
}

const Method* Class::GetMethod(const std::string& /*name*/) const {
    // Заглушка. Реализуйте метод самостоятельно
    return nullptr;
}

[[nodiscard]] inline const std::string& Class::GetName() const {
    // Заглушка. Реализуйте метод самостоятельно.
    throw std::runtime_error("Not implemented"s);
}

void Class::Print(ostream& /*os*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
}

void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
    os << (GetValue() ? "True"sv : "False"sv);
}

bool Equal(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool Less(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for less"s);
}

bool NotEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool Greater(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool LessOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool GreaterOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

}  // namespace runtime