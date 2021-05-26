//
// Created by azakharov on 4/4/2021.
//

#include "document.h"

#include <string>

namespace sprint_4::server {

std::ostream &operator<<(std::ostream &os, const Document &document) {
    using namespace std::literals;

    return os << "{ document_id = "s << document.id << ", "s
              << "relevance = "s << document.relevance << ", "s
              << "rating = "s << document.rating << " }"s;
}

}  // namespace sprint_4::server
