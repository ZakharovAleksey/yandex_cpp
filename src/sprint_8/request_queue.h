#pragma once

#include <deque>

#include "search_server.h"

namespace sprint_8::server {

class RequestQueue {
public:  // Constructors
    explicit RequestQueue(const SearchServer& search_server) : server_(search_server) {}

public:  // Methods
    template <typename DocumentFilterFunction>
    std::vector<Document> AddFindRequest(const std::string& raw_query,
                                         DocumentFilterFunction document_filter_function) {
        std::vector<Document> response = server_.FindTopDocuments(raw_query, document_filter_function);
        UpdateRequestsInformation(response);
        requests_.emplace_back(raw_query, response.size());

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
        int found_documents_count{0};

        QueryResult(const std::string& query, int found_documents_count)
            : query(query), found_documents_count(found_documents_count) {}

        [[nodiscard]] bool IsEmpty() const {
            return found_documents_count == 0;
        }
    };

private:  // Methods
    void UpdateRequestsInformation(const std::vector<Document>& response);

private:  // Fields
    std::deque<QueryResult> requests_;
    const SearchServer& server_;
    int64_t empty_responses_count_{0};
};
}  // namespace sprint_8::server