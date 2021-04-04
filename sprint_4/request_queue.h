//
// Created by azakharov on 4/4/2021.
//

#pragma once

#include <deque>

#include "document.h"
#include "search_server.h"

namespace sprint_4::server {

class RequestQueue {
   public:  // Constructors
    explicit RequestQueue(const SearchServer& search_server) : server_(search_server) {}

   public:  // Methods
    template <typename DocumentFilterFunction>
    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentFilterFunction document_filter_function) {
        // Remove request from the deque if it is too old
        if (requests_.size() == kSecondsInDay)
            requests_.pop_front();

        std::vector<Document> response = server_.FindTopDocuments(raw_query, document_filter_function);
        requests_.emplace_back(response, response.empty());

        return response;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentStatus document_status = DocumentStatus::ACTUAL);

    [[nodiscard]] int GetNoResultRequests() const;

   private:  // Constants
    const static int kSecondsInDay{1440};

   private:  // Types
    struct QueryResult {
        std::vector<Document> response;
        bool is_empty{false};

        QueryResult() = default;
        QueryResult(const std::vector<Document>& response, bool is_response_empty)
            : response(response), is_empty(is_response_empty) {}
    };

   private:  // Fields
    std::deque<QueryResult> requests_;
    const SearchServer& server_;
};
}  // namespace sprint_4::server