//
// Created by azakharov on 3/8/2021.
//

#include "testing_framework.h"

#include <iterator>

namespace unit_test {
TestRunner::~TestRunner() {
    std::cerr << "\nTotal tests number: "s << std::to_string(test_count_) << " Failed tests number: "s
              << std::to_string(failed_test_names_.size()) << std::endl;

    if (!failed_test_names_.empty()) {
        std::cerr << "Failed tests: "s;
        std::copy(failed_test_names_.begin(), failed_test_names_.end(),
                  std::ostream_iterator<std::string>(std::cerr, " "));
    } else
        std::cerr << "All tests passed without errors"s << std::endl;
}

void AssertTrue(const bool condition, const std::string &expression, const std::string &file,
                const std::string &function, unsigned line, const std::string &hint) {
    std::stringstream error;

    if (!condition) {
        error << file << "("s << line << "): "s << function << ": "s;
        error << "ASSERT("s << expression << ") failed."s;

        if (!hint.empty())
            error << " Hint: "s << hint;

        error << std::endl;
        throw TestFailureException(error.str());
    }
}

}  // namespace unit_test