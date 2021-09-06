//
// Created by azakharov on 5/23/2021.
//

#include "sprint_7_tasks.h"

namespace intermediate {
void Editor::Left() {
    current_position_ = current_position_ == text_.begin() ? current_position_ : std::prev(current_position_);
}

void Editor::Right() {
    current_position_ = current_position_ == text_.end() ? current_position_ : std::next(current_position_);
}

void Editor::Insert(char token) {
    text_.insert(current_position_, token);
}

void Editor::Cut(size_t tokens) {
    auto last_available_position = CopyToBuffer(tokens);
    text_.erase(current_position_, last_available_position);
    current_position_ = last_available_position;
}

void Editor::Copy(size_t tokens) {
    CopyToBuffer(tokens);
}

void Editor::Paste() {
    text_.insert(current_position_, buffer_.begin(), buffer_.end());
}

[[nodiscard]] std::string Editor::GetText() const {
    return {text_.begin(), text_.end()};
}

std::list<char>::iterator Editor::CopyToBuffer(size_t tokens) {
    buffer_.clear();
    buffer_.reserve(tokens);

    auto last_available_position = current_position_;
    for (size_t position_id = 0; position_id != tokens; ++position_id) {
        if (last_available_position == text_.end())
            break;
        buffer_.push_back(*last_available_position);
        last_available_position = std::next(last_available_position);
    }

    return last_available_position;
}

void Translator::Add(std::string_view source, std::string_view target) {
    words_.push_front({source.begin(), source.end()});
    words_.push_front({target.begin(), target.end()});

    source_[*std::next(words_.begin())] = *words_.begin();
    destination_[*words_.begin()] = *std::next(words_.begin());
}

[[nodiscard]] std::string_view Translator::TranslateForward(std::string_view source) const {
    return source_.count(source) > 0 ? source_.at(source) : std::string_view();
}
[[nodiscard]] std::string_view Translator::TranslateBackward(std::string_view target) const {
    return destination_.count(target) > 0 ? destination_.at(target) : std::string_view();
}

}  // namespace intermediate