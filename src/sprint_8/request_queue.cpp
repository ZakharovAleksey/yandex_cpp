//
// Created by azakharov on 4/4/2021.
//

#include "request_queue.h"

#include <numeric>

namespace sprint_8::server {

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus document_status) {
    return AddFindRequest(raw_query,
                          [document_status]([[maybe_unused]] int document_id, DocumentStatus status,
                                            [[maybe_unused]] int rating) { return status == document_status; });
}

int RequestQueue::GetNoResultRequests() const {
    return empty_responses_count_;
}

void RequestQueue::UpdateRequestsInformation(const std::vector<Document>& response) {
    // Remove request from the deque if it is too old
    if (requests_.size() == kMinutesInDay) {
        // If the front response was empty - update number of empty responses
        if (requests_.front().IsEmpty())
            --empty_responses_count_;

        requests_.pop_front();
    }

    if (response.empty())
        ++empty_responses_count_;
}

}  // namespace sprint_8::server
