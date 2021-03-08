//
// Created by azakharov on 3/8/2021.
//

#include "testing_framework.h"

namespace unit_test {
    void AssertTrue(const bool condition,
                    const std::string &expression,
                    const std::string &file,
                    const std::string &function,
                    unsigned line,
                    const std::string &hint) {
        using std::cerr;

        if (!condition) {
            cerr << file << "("s << line << "): "s << function << ": "s;
            cerr << "ASSERT("s << expression << ") failed."s;

            if (!hint.empty())
                std::cerr << " Hint: "s << hint;

            cerr << std::endl;
            abort();
        }
    }

    void RunTest(std::function<void()> function, const std::string &function_name) {
        function();
        std::cerr << function_name << " OK" << std::endl;
    }
}