//
// Created by azakharov on 4/4/2021.
//

#include "string_processing.h"

#include <algorithm>

namespace sprint_5::server::utils {

bool IsValidWord(const std::string &word) {
    // A valid word must not contain special characters in range [0, 31]
    return std::none_of(word.begin(), word.end(), [](char symbol) { return symbol >= '\0' && symbol < ' '; });
}

std::vector<std::string> SplitIntoWords(const std::string & text) {
    std::vector<std::string> words;
    std::string word;
    for (const char symbol : text) {
        if (symbol == ' ') {
            words.push_back(word);
            word = "";
        } else
            word += symbol;
    }

    words.emplace_back(word);
    return words;
}

}  // namespace sprint_5::server::utils
