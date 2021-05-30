#pragma once

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace sprint_5::server::utils {

bool IsValidWord(std::string_view word);

std::vector<std::string_view> SplitIntoWords(std::string_view text);

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings) {
    using namespace std::literals;

    std::set<std::string> non_empty_strings;

    for (std::string_view current_string : strings) {
        if (!IsValidWord(current_string))
            throw std::invalid_argument("Invalid word in the document: "s + current_string.data());

        if (!current_string.empty())
            non_empty_strings.insert(std::string(current_string));
    }

    return non_empty_strings;
}

}  // namespace sprint_5::server::utils
