//
// Created by azakharov on 5/23/2021.
//

#pragma once

#include <forward_list>
#include <list>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace intermediate {
/*
 * Text editor with listed below operations
 */
class Editor {
public:  // Constructor
    Editor() = default;

public:  // Methods
    void Left();

    void Right();

    void Insert(char token);

    void Cut(size_t tokens = 1);

    void Copy(size_t tokens = 1);

    void Paste();

    [[nodiscard]] std::string GetText() const;

private:  // Methods
    std::list<char>::iterator CopyToBuffer(size_t tokens = 1);

private:  // Fields
    std::list<char> text_;
    std::list<char>::iterator current_position_{text_.begin()};
    std::string buffer_;
};

/*
 * Translation with string_view.
 * Key idea: store words in the container, which does not invalidate it's iterators on the push_back() to it.
 * This is list: so we store data in list, and return string_views to stored strings in the list ;)
 */
class Translator {
public:  // Types
    using Dictionary = std::map<std::string_view, std::string_view>;

public:  // Methods
    void Add(std::string_view source, std::string_view target);

    [[nodiscard]] std::string_view TranslateForward(std::string_view source) const;
    [[nodiscard]] std::string_view TranslateBackward(std::string_view target) const;

private:  // Fields
    std::forward_list<std::string> words_;
    Dictionary source_;
    Dictionary destination_;
};

/*
 * In this section you could find function, which splits words on sentences without copying them
 * Use only std::make_move_iterator()
 */
template <typename Token>
using Sentence = std::vector<Token>;

template <typename TokenForwardIt>
TokenForwardIt FindSentenceEnd(TokenForwardIt tokens_begin, TokenForwardIt tokens_end) {
    const TokenForwardIt before_sentence_end =
        adjacent_find(tokens_begin, tokens_end, [](const auto& left_token, const auto& right_token) {
            return left_token.IsEndSentencePunctuation() && !right_token.IsEndSentencePunctuation();
        });
    return before_sentence_end == tokens_end ? tokens_end : next(before_sentence_end);
}

template <typename Token>
std::vector<Sentence<Token>> SplitIntoSentences(std::vector<Token> tokens) {
    std::vector<Sentence<Token>> sentences;

    auto sentence_begin = tokens.begin();
    while (sentence_begin != tokens.end()) {
        auto sentence_end = FindSentenceEnd(sentence_begin, tokens.end());
        sentences.push_back({std::make_move_iterator(sentence_begin), std::make_move_iterator(sentence_end)});
        sentence_begin = sentence_end;
    }

    return sentences;
}

/*
 * Josephus Permutation task without copying.
 */
template <typename RandomIt>
void MakeJosephusPermutation(RandomIt first, RandomIt last, uint32_t step_size) {
    std::vector<typename RandomIt::value_type> pool(std::make_move_iterator(first), std::make_move_iterator(last));
    size_t cur_pos = 0;
    while (!pool.empty()) {
        *(first++) = std::move(pool[cur_pos]);
        pool.erase(pool.begin() + cur_pos);
        if (pool.empty()) {
            break;
        }
        cur_pos = (cur_pos + step_size - 1) % pool.size();
    }
}

}  // namespace intermediate