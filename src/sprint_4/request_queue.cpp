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
    return empty_responses_count_;
}

}  // namespace sprint_4::server
