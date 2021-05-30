//
// Created by azakharov on 4/4/2021.
//

#include "string_processing.h"

#include <algorithm>

namespace sprint_5::server::utils {

bool IsValidWord(std::string_view word) {
    // A valid word must not contain special characters in range [0, 31]
    return std::none_of(word.begin(), word.end(), [](char symbol) { return symbol >= '\0' && symbol < ' '; });
}

std::vector<std::string_view> SplitIntoWords(std::string_view text) {
    std::vector<std::string_view> words;
    unsigned int word_begin{0u};

    while (word_begin <= text.length()){
        unsigned int word_end = text.find(' ', word_begin);

        words.push_back(text.substr(word_begin, word_end - word_begin));
        word_begin = word_end == std::string_view::npos ? word_end : word_end + 1;
    }

    return words;
}

}  // namespace sprint_5::server::utils
