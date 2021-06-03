#pragma once

#include <chrono>
#include <iostream>
#include <string>

namespace sprint_8::server::utils {

class LogDuration {
public:  // Declarations
    using Clock = std::chrono::steady_clock;

public:  // Constructors
    LogDuration(std::string_view code_block_name, std::ostream& out_stream)
        : code_block_name_(std::move(code_block_name)), out_stream_(out_stream) {}

public:  // Destructors
    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto duration = end_time - start_time_;
        out_stream_ << code_block_name_ << ": "s << duration_cast<milliseconds>(duration).count() << " ms"s
                    << std::endl;
    }

private:  // Fields
    const std::string code_block_name_;
    std::ostream& out_stream_;
    const Clock::time_point start_time_ = Clock::now();
};

}  // namespace sprint_8::server::utils

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(code_block, out_stream) \
    sprint_8::server::utils::LogDuration UNIQUE_VAR_NAME_PROFILE(code_block, out_stream)
