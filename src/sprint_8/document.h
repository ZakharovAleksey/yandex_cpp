#pragma once

#include <iostream>

namespace sprint_8::server {

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating) : id(id), relevance(relevance), rating(rating) {}

    int id{0};
    double relevance{0.};
    int rating{0};
};

std::ostream &operator<<(std::ostream &os, const Document &document);

}  // namespace sprint_8::server