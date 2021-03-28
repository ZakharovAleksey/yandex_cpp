//
// Created by azakharov on 3/7/2021.
//

#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
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

class TestFailureException : public std::logic_error {
   public:
    explicit TestFailureException(const std::string &what) : std::logic_error(what) {}

   public:
    [[nodiscard]] const char *what() const noexcept final {
        return std::logic_error::what();
    }
};

class TestRunner {
   public:  // Constructor & Destructor
    TestRunner() = default;
    ~TestRunner();

   public:
    template <class TestFunction>
    void RunTest(TestFunction test, const std::string &test_name) {
        ++test_count_;

        try {
            test();
            std::cerr << test_name << ": OK"s << std::endl;
        } catch (TestFailureException &failed_test) {
            failed_test_names_.emplace_back(test_name);
            std::cerr << test_name << ": FAILED"s << std::endl;
            std::cerr << failed_test.what();
        } catch (...) {
            failed_test_names_.emplace_back(test_name);
            std::cerr << test_name << ": FAILED "s << std::endl;
            std::cerr << "Reason: unexpected exception in test function"s << std::endl;
        }
    }

   private:
    int test_count_{0};
    std::vector<std::string> failed_test_names_;
};

inline TestRunner GlobalTestRunner;  //> Global test runner, which starts all tests execution

template <typename LeftValueType, typename RightValueType>
void AssertEqual(const LeftValueType &left, const RightValueType &right, const std::string &left_string,
                 const std::string &right_string, const std::string &file, const std::string &function, unsigned line,
                 const std::string &hint) {
    std::stringstream error;

    if (left != right) {
        error << std::boolalpha;
        error << file << "("s << line << "): "s << function << ": "s;
        error << "ASSERT_EQUAL("s << left_string << ", "s << right_string << ") failed: "s;
        error << left << " != "s << right << "."s;

        if (!hint.empty())
            error << " Hint: "s << hint;

        error << std::endl;
        throw TestFailureException(error.str());
    }
}

void AssertTrue(const bool condition, const std::string &expression, const std::string &file,
                const std::string &function, unsigned line, const std::string &hint);

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

#define RUN_TEST(test) unit_test::GlobalTestRunner.RunTest((test), #test)