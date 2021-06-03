//
// Created by azakharov on 6/2/2021.
//

#pragma once

#include <functional>

#include "search_server.h"

namespace sprint_5::server {

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server,
                                                  const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> responses(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), responses.begin(),
                   [&search_server](const std::string& query) { return search_server.FindTopDocuments(query); });
    return responses;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
    auto responses = ProcessQueries(search_server, queries);
    size_t total_documents_count =
        std::transform_reduce(std::execution::par, responses.begin(), responses.end(), 0, std::plus<>(),
                              [](const auto& response) { return response.size(); });

    std::vector<Document> result;
    result.reserve(total_documents_count);
    for (auto response : responses)
        std::move(response.begin(), response.end(), std::back_inserter(result));

    return result;
}

}  // namespace sprint_5::server