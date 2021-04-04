//
// Created by azakharov on 4/4/2021.
//

#include "search_server.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>

namespace sprint_4::server {

using namespace std::literals;
using namespace utils;

// SearchServer class methods below

void SearchServer::SetStopWords(const std::string &text) {
    for (const std::string &word : SplitIntoWords(text))
        stop_words_.insert(word);
}

void SearchServer::AddDocument(int document_id, const std::string &document, DocumentStatus status,
                               const std::vector<int> &ratings) {
    if (auto [is_correct, error_message] = CheckDocumentInput(document_id, document); !is_correct)
        throw std::invalid_argument(error_message);

    const std::vector<std::string> words = SplitDocumentIntoNoWords(document);
    const double inverse_word_count = 1. / words.size();

    for (const std::string &word : words)
        word_to_document_frequency_[word][document_id] += inverse_word_count;

    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.emplace_back(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string &raw_query,
                                                     DocumentStatus document_status) const {
    return FindTopDocuments(raw_query, [document_status](int document_id, DocumentStatus status, int rating) {
        return status == document_status;
    });
}

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(documents_.size());
}

int SearchServer::GetDocumentId(int index) const {
    return document_ids_.at(index);
}

SearchServer::WordsInDocumentInfo SearchServer::MatchDocument(const std::string &raw_query, int document_id) const {
    const Query query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;

    for (const std::string &word : query.plus_words) {
        if (word_to_document_frequency_.count(word) == 0)
            continue;

        if (word_to_document_frequency_.at(word).count(document_id))
            matched_words.push_back(word);
    }

    for (const std::string &word : query.minus_words) {
        if (word_to_document_frequency_.count(word) == 0)
            continue;

        if (word_to_document_frequency_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }

    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(const std::string &word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitDocumentIntoNoWords(const std::string &text) const {
    std::vector<std::string> words;

    for (const std::string &word : SplitIntoWords(text)) {
        if (!IsValidWord(word))
            throw std::invalid_argument("Invalid word in the document: "s + word);

        if (!IsStopWord(word))
            words.push_back(word);
    }

    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int> &ratings) {
    return !ratings.empty() ? std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size()) : 0;
}

bool SearchServer::ParseQueryWord(std::string word, QueryWord &query_word) const {
    query_word = {};

    assert(!word.empty() && "Input word in query could not be empty");

    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }

    // Check word is not '-' or starts from '--'
    if (word.empty() || word[0] == '-' || !IsValidWord(word))
        return false;

    query_word = {word, is_minus, IsStopWord(word)};
    return true;
}

SearchServer::Query SearchServer::ParseQuery(const std::string &query_text) const {
    Query query;
    for (const std::string &word : SplitIntoWords(query_text)) {
        QueryWord query_word;
        if (!ParseQueryWord(word, query_word))
            throw std::invalid_argument("Invalid word in the query: "s + word);

        if (!query_word.is_stop) {
            if (query_word.is_minus)
                query.minus_words.insert(query_word.data);
            else
                query.plus_words.insert(query_word.data);
        }
    }

    return query;
}

std::pair<bool, std::string> SearchServer::CheckDocumentInput(int document_id, const std::string &document) {
    if (document_id < 0)
        return {false, "Negative document index is not expected"s};

    if (documents_.count(document_id) > 0)
        return {false, "Document with index # " + std::to_string(document_id) +
                           " is already exists. Duplicates are not allowed"s};

    if (document.empty())
        return {false, "Empty document is not allowed"s};

    return {true, ""s};
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFrequency(const std::string &word) const {
    assert(word_to_document_frequency_.count(word) != 0 &&
           "Expected word is not found in documents - could not calculate frequency");

    auto documents_with_word = word_to_document_frequency_.at(word);
    assert(!documents_with_word.empty() && "Expected word is not found in documents - could not calculate frequency");

    return log(GetDocumentCount() * 1. / documents_with_word.size());
}

}  // namespace sprint_4::server
