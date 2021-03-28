//
// Created by azakharov on 3/7/2021.
//

#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace unit_test {
using namespace std::string_literals;

template <class Container>
std::ostream &PrintContainer(std::ostream &os, const Container &container) {
    size_t index{0u};
    for (const auto &value : container) {
        os << value;
        if (index++ != container.size() - 1)
            os << ", "s;
    }

    return os;
}

template <class Type>
std::ostream &operator<<(std::ostream &os, const std::vector<Type> &container) {
    os << "["s;
    PrintContainer(os, container);
    os << "]"s;

    return os;
}

template <class Type>
std::ostream &operator<<(std::ostream &os, const std::set<Type> &container) {
    os << "{"s;
    PrintContainer(os, container);
    os << "}"s;

    return os;
}

template <class KeyType, class ValueType>
std::ostream &operator<<(std::ostream &os, const std::pair<KeyType, ValueType> &values_pair) {
    os << values_pair.first << ": "s << values_pair.second;

    return os;
}

template <class KeyType, class ValueType>
std::ostream &operator<<(std::ostream &os, const std::map<KeyType, ValueType> &container) {
    os << "{"s;
    PrintContainer(os, container);
    os << "}"s;

    return os;
}

template <typename LeftValueType, typename RightValueType>
void AssertEqual(const LeftValueType &left, const RightValueType &right, const std::string &left_string,
                 const std::string &right_string, const std::string &file, const std::string &function, unsigned line,
                 const std::string &hint) {
    using std::cerr;
    if (left != right) {
        cerr << std::boolalpha;
        cerr << file << "("s << line << "): "s << function << ": "s;
        cerr << "ASSERT_EQUAL("s << left_string << ", "s << right_string << ") failed: "s;
        cerr << left << " != "s << right << "."s;

        if (!hint.empty())
            cerr << " Hint: "s << hint;

        cerr << std::endl;
        abort();
    }
}

void AssertTrue(const bool condition, const std::string &expression, const std::string &file,
                const std::string &function, unsigned line, const std::string &hint);

void RunTest(const std::function<void()>& function, const std::string &function_name);
}  // namespace unit_test

#define ASSERT_EQUAL(left, right) \
    unit_test::AssertEqual((left), (right), #left, #right, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(left, right, hint) \
    unit_test::AssertEqual((left), (right), #left, #right, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT(condition) unit_test::AssertTrue(!!(condition), #condition, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(condition, hint) \
    unit_test::AssertTrue(!!(condition), #condition, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT_THROW(statement, expected_exception) \
    bool is_throw{false};                           \
    try {                                           \
        statement;                                  \
    } catch (expected_exception & e) {              \
        is_throw = true;                            \
    }                                               \
    unit_test::AssertTrue(!!(is_throw), #statement, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_THROW_HINT(statement, expected_exception, hint) \
    bool is_throw{false};                                      \
    try {                                                      \
        statement;                                             \
    } catch (expected_exception & e) {                         \
        is_throw = true;                                       \
    }                                                          \
    unit_test::AssertTrue(!!(is_throw), #statement, __FILE__, __FUNCTION__, __LINE__, (hint))

#define RUN_TEST(test_function) unit_test::RunTest((test_function), #test_function)
