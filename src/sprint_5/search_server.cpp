//
// Created by azakharov on 4/4/2021.
//

#include "search_server.h"

#include <cmath>
#include <iostream>
#include <numeric>

namespace sprint_5::server {

using namespace std::literals;
using namespace utils;

template <class Container>
void RemoveByDocumentId(Container &container, DocumentId index) {
    if (auto position = container.find(index); position != container.end())
        container.erase(position);
}

// SearchServer class methods below

void SearchServer::SetStopWords(const std::string &text) {
    for (const std::string &word : SplitIntoWords(text))
        stop_words_.insert(word);
}

void SearchServer::AddDocument(int document_id, const std::string &document, DocumentStatus status,
                               const std::vector<int> &ratings) {
    if (const auto &error_message = CheckDocumentInput(document_id, document);
        error_message && !error_message->empty()) {
        throw std::invalid_argument(*error_message);
    }

    const std::vector<std::string> words = SplitDocumentIntoNoWords(document);
    const double inverse_word_count = 1. / words.size();

    for (const std::string &word : words) {
        word_to_document_frequency_[word][document_id] += inverse_word_count;
        words_frequency_by_documents_[document_id][word] += inverse_word_count;
    }

    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id);
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

std::optional<std::string> SearchServer::CheckDocumentInput(int document_id, const std::string &document) {
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
double SearchServer::ComputeWordInverseDocumentFrequency(const std::string &word) const {
    return log(GetDocumentCount() * 1. / word_to_document_frequency_.at(word).size());
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

const std::map<Word, double> &SearchServer::GetWordFrequencies(DocumentId index) const {
    static std::map<Word, double> result;
    result.clear();

    if (auto words_frequency = words_frequency_by_documents_.find(index);
        words_frequency != words_frequency_by_documents_.end()) {
        result.insert(words_frequency->second.begin(), words_frequency->second.end());
    }

    return result;
}

void SearchServer::RemoveDocument(DocumentId index) {
    auto document_position = document_ids_.find(index);

    if (document_position == document_ids_.end())
        return;

    document_ids_.erase(document_position);

    for (const auto &[word, _] : words_frequency_by_documents_.at(index))
        RemoveByDocumentId(word_to_document_frequency_.at(word), index);

    RemoveByDocumentId(documents_, index);
    RemoveByDocumentId(words_frequency_by_documents_, index);
}

void RemoveDuplicates(SearchServer &search_server) {
    std::map<std::set<Word>, DocumentId> storage;
    std::vector<DocumentId> indexes_for_removal;

    auto get_distinct_words = [&](const std::map<Word, double> &word_frequencies) {
        std::set<Word> result;
        for (const auto &[word, _] : word_frequencies)
            result.insert(word);
        return result;
    };

    for (const DocumentId index : search_server) {
        const auto &word_frequencies = search_server.GetWordFrequencies(index);
        auto storage_key = get_distinct_words(word_frequencies);

        if (auto position = storage.find(storage_key); position != storage.end()) {
            DocumentId index_to_remove = std::max(index, position->second);
            position->second = std::min(index, position->second);

            indexes_for_removal.emplace_back(index_to_remove);
        } else
            storage.insert({storage_key, index});
    }

    for (DocumentId index : indexes_for_removal)
        search_server.RemoveDocument(index);
}

}  // namespace sprint_5::server
