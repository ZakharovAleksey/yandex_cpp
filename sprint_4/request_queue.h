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
        if (requests_.size() == kMinutesInDay) {
            // If the front response was empty - update number of empty responses
            if (requests_.front().IsEmpty())
                --empty_responses_count_;

            requests_.pop_front();
        }

        std::vector<Document> response = server_.FindTopDocuments(raw_query, document_filter_function);
        requests_.emplace_back(raw_query, response.size());

        if (response.empty())
            ++empty_responses_count_;

        return response;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentStatus document_status = DocumentStatus::ACTUAL);

    [[nodiscard]] int GetNoResultRequests() const;

  private:  // Constants
    const static int kMinutesInDay{1440};

  private:  // Types
    struct QueryResult {
        std::string query;
        size_t found_documents_count{0u};

        QueryResult(const std::string& query, size_t found_documents_count)
            : query(query), found_documents_count(found_documents_count) {}

        bool IsEmpty() const {
            return found_documents_count == 0u;
        }
    };

  private:  // Fields
    std::deque<QueryResult> requests_;
    const SearchServer& server_;
    int empty_responses_count_{0};
};
}  // namespace sprint_4::server