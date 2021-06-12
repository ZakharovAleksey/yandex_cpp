//
// Created by azakharov on 4/4/2021.
//

#include "search_server.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <numeric>

namespace sprint_8::server {

using namespace std::literals;
using namespace utils;

// SearchServer class methods below

void SearchServer::SetStopWords(std::string_view text) {
    for (std::string_view word : SplitIntoWords(text))
        stop_words_.insert(std::string(word));
}

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status,
                               const std::vector<int> &ratings) {
    if (const auto &error_message = CheckDocumentInput(document_id, document);
        error_message && !error_message->empty()) {
        throw std::invalid_argument(*error_message);
    }

    const std::vector<std::string_view> words = SplitDocumentIntoNoWords(document);
    const double inverse_word_count = 1. / words.size();

    for (std::string_view word : words) {
        word_to_document_frequency_[std::string(word)][document_id] += inverse_word_count;
        const auto position = word_to_document_frequency_.find(word);
        words_frequency_by_documents_[document_id][position->first] += inverse_word_count;
    }

    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus document_status) const {
    // clang-format off
    return FindTopDocuments(raw_query,
                            [document_status](
                                [[maybe_unused]] int document_id, DocumentStatus status, [[maybe_unused]] int rating) {
                                return status == document_status;
                            });
    // clang-format on
}

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(documents_.size());
}

SearchServer::WordsInDocumentInfo SearchServer::MatchDocument(std::string_view raw_query, int document_id) const {
    const Query query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;
    matched_words.reserve(query.plus_words.size() + query.minus_words.size());

    for (std::string_view word : query.plus_words) {
        const auto word_position = word_to_document_frequency_.find(word);
        if (word_position == word_to_document_frequency_.cend())
            continue;

        if (word_position->second.count(document_id))
            matched_words.push_back(word_position->first);
    }

    for (std::string_view word : query.minus_words) {
        const auto word_position = word_to_document_frequency_.find(word);
        if (word_position == word_to_document_frequency_.cend())
            continue;

        if (word_position->second.count(document_id)) {
            matched_words.clear();
            break;
        }
    }

    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(std::string_view word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string_view> SearchServer::SplitDocumentIntoNoWords(std::string_view text) const {
    std::vector<std::string_view> words;

    for (std::string_view word : SplitIntoWords(text)) {
        if (!IsValidWord(word))
            throw std::invalid_argument("Invalid word in the document: "s + std::string(word));

        if (!IsStopWord(word))
            words.push_back(word);
    }

    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int> &ratings) {
    return !ratings.empty() ? std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size()) : 0;
}

bool SearchServer::ParseQueryWord(std::string_view word, QueryWord &query_word) const {
    query_word = {};

    if (word.empty())
        return false;

    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }

    if (word.empty() || word[0] == '-' || !IsValidWord(word))  //> Check word is not '-' or starts from '--'
        return false;

    query_word = {word, is_minus, IsStopWord(word)};
    return true;
}

SearchServer::Query SearchServer::ParseQuery(std::string_view query_text) const {
    Query query;

    for (std::string_view word : SplitIntoWords(query_text)) {
        QueryWord query_word;
        if (!ParseQueryWord(word, query_word))
            throw std::invalid_argument("Invalid word in the query: "s + std::string(word));

        if (!query_word.is_stop) {
            if (query_word.is_minus)
                query.minus_words.insert(query_word.data);
            else
                query.plus_words.insert(query_word.data);
        }
    }

    return query;
}

std::optional<std::string> SearchServer::CheckDocumentInput(int document_id, std::string_view document) {
    if (document_id < 0)
        return "Negative document index is not expected"s;

    if (documents_.count(document_id) > 0)
        return "Document with index # " + std::to_string(document_id) +
               " is already exists. Duplicates are not allowed"s;

    if (document.empty())
        return "Empty document is not allowed"s;

    return std::nullopt;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFrequency(std::string_view word) const {
    const auto word_position = word_to_document_frequency_.find(word);
    assert(word_position != word_to_document_frequency_.cend() &&
           "Could not compute inverse document frequency if word is not in the document");

    return log(GetDocumentCount() * 1. / word_position->second.size());
}

std::set<int>::iterator SearchServer::begin() {
    return document_ids_.begin();
}

std::set<int>::const_iterator SearchServer::begin() const {
    return document_ids_.begin();
}

std::set<int>::iterator SearchServer::end() {
    return document_ids_.end();
}

std::set<int>::const_iterator SearchServer::end() const {
    return document_ids_.end();
}

const std::map<std::string_view, double> &SearchServer::GetWordFrequencies(DocumentId index) const {
    static std::map<std::string_view, double> default_option;
    const auto words_frequencies_position = words_frequency_by_documents_.find(index);

    return (words_frequencies_position == words_frequency_by_documents_.cend()) ? default_option
                                                                                : words_frequencies_position->second;
}

void SearchServer::RemoveDocument(DocumentId index) {
    auto document_position = document_ids_.find(index);
    if (document_position == document_ids_.end())
        return;
    document_ids_.erase(document_position);

    for (auto [word, _] : words_frequency_by_documents_.at(index)) {
        auto position = word_to_document_frequency_.find(word);
        position->second.erase(index);
    }

    documents_.erase(index);
    words_frequency_by_documents_.erase(index);
}

void RemoveDuplicates(SearchServer &search_server) {
    std::map<std::set<Word>, DocumentId> storage;
    std::vector<DocumentId> indices_for_removal;

    for (const auto index : search_server) {
        const auto &word_frequencies = search_server.GetWordFrequencies(index);
        std::set<Word> storage_key;
        std::transform(word_frequencies.begin(), word_frequencies.end(),
                       std::inserter(storage_key, storage_key.begin()),
                       [](const auto &item) { return std::string(item.first); });

        // In std::set<> keys are in increasing order. Ao, if we found the same 'storage_key' in the storage -
        // we should remove current documents, because it has higher document index
        if (storage.count(storage_key) > 0) {
            indices_for_removal.emplace_back(index);
        } else {
            storage.insert({storage_key, index});
        }
    }

    for (auto index : indices_for_removal)
        search_server.RemoveDocument(index);
}

}  // namespace sprint_8::server
