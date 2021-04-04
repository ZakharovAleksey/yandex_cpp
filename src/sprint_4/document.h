//
// Created by azakharov on 4/4/2021.
//

#pragma once

#include <iostream>

namespace sprint_4::server {

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

}  // namespace sprint_4::server