//
// Created by azakharov on 4/4/2021.
//

#include "request_queue.h"

#include <numeric>

namespace sprint_4::server {

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus document_status) {
    return AddFindRequest(raw_query, [document_status](int document_id, DocumentStatus status, int rating) {
        return status == document_status;
    });
}

int RequestQueue::GetNoResultRequests() const {
    return std::accumulate(requests_.begin(), requests_.end(), 0,
                           [](int& current_sum, const QueryResult& query_result) {
                               return query_result.is_empty ? ++current_sum : current_sum;
                           });
}

}  // namespace sprint_4::server
