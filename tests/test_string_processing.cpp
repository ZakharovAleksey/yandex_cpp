//
// Created by azakharov on 4/4/2021.
//

#include <gtest/gtest.h>

#include "../src/sprint_8/string_processing.h"

using namespace sprint_8::server;
using namespace std::literals;

TEST(StringProcessingFunctions, CheckInvalidWords) {
    const std::string prefix{"template"};
    for (int symbol_id = 0; symbol_id <= 31; ++symbol_id) {
        std::string invalid_word = prefix + static_cast<char>(symbol_id);
        bool isValid = utils::IsValidWord(invalid_word);
        EXPECT_TRUE(!isValid) << "Word "s << invalid_word << " expected to be invalid due to "s
                              << static_cast<char>(symbol_id) << " symbol"s;
    }
}

TEST(StringProcessingFunctions, CheckSplitIntoWords) {
    EXPECT_TRUE(utils::SplitIntoWords(""s).at(0).empty()) << "Empty string could not be split into words";

    std::map<std::string_view, std::vector<std::string_view>> input_output = {
        {"first", {"first"}},
        {"first second third", {"first", "second", "third"}},
        {"first  second   third", {"first", "", "second", "", "", "third"}}};

    for (auto [text, words] : input_output)
        EXPECT_EQ(utils::SplitIntoWords(text), words) << "Incorrect split into words"s;
}

TEST(StringProcessingFunctions, CheckMakeUniqueNonEmptyWords) {
    const std::set<std::string, std::less<>> expected_words = {"first"s, "second"s};

    std::vector<std::string> container_with_empty_string = {"first"s, ""s, "second"s, ""s, ""s};
    auto actual_words = utils::MakeUniqueNonEmptyStrings(container_with_empty_string);
    EXPECT_EQ(actual_words.size(), 2) << "MakeUniqueNonEmptyWords() function should exclude empty strings"s;
    EXPECT_EQ(actual_words, expected_words) << "MakeUniqueNonEmptyWords() function should exclude empty strings"s;

    std::vector<std::string> container_repeated_words = {"first"s, "second"s, "first"s, "first"s, "second"s};
    actual_words = utils::MakeUniqueNonEmptyStrings(container_repeated_words);
    EXPECT_EQ(actual_words.size(), 2) << "MakeUniqueNonEmptyWords() function should exclude duplicates"s;
    EXPECT_EQ(actual_words, expected_words) << "MakeUniqueNonEmptyWords() function should exclude empty strings"s;
}