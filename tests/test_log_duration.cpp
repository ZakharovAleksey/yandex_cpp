//
// Created by azakharov on 4/19/2021.
//

#include <gtest/gtest.h>

#include <regex>
#include <thread>

#include "../src/sprint_8/log_duration.h"

using namespace sprint_8::server::utils;
using namespace std::literals;
using namespace std::chrono_literals;

TEST(LogDurationClass, LogDurationMeasureExactTime) {
    const int expected_time{2000};  //> The same time as in sleep_for function
    const std::string code_block_name{"test-function"s};

    std::stringstream output_stream;
    {
        LogDuration timer(code_block_name, output_stream);
        std::this_thread::sleep_for(2000ms);
    }
    const std::string actual_message = output_stream.str();
    std::regex number_pattern{"\\d{4}"};
    std::smatch match;

    EXPECT_TRUE(std::regex_search(actual_message.begin(), actual_message.end(), match, number_pattern))
        << "Logger should provide execution time in output message"s;

    const int actual_time = std::stoi(match[0]);

    EXPECT_NEAR(expected_time, actual_time, expected_time / 10)
        << "Logger should provide correct time measurement"s;

    EXPECT_EQ(actual_message, code_block_name + ": " + std::to_string(actual_time) + " ms\n")
        << "Logger should provide correct message template"s;
}
