//
// Created by azakharov on 4/4/2021.
//

#pragma once

#include <set>
#include <string>
#include <vector>

namespace sprint_4::server::utils {

bool IsValidWord(const std::string &word);

std::vector<std::string> SplitIntoWords(const std::string &text);

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings) {
    std::set<std::string> non_empty_strings;

    for (const auto &current_string : strings) {
        if (!IsValidWord(current_string))
            throw std::invalid_argument("Invalid word in the document: "s + word);
        
        if (!current_string.empty())
            non_empty_strings.insert(current_string);
    }

    return non_empty_strings;
}

}  // namespace sprint_4::server::utils
