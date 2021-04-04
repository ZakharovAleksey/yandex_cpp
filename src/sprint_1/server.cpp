//
// Created by azakharov on 2/19/2021.
//

#include "server.h"

#include <cmath>
#include <iostream>
#include <numeric>

namespace sprint_1::server {

using namespace std::literals;
using std::string;
using std::vector;

std::ostream &operator<<(std::ostream &os, const Document &document) {
    return os << "{ document_id = "s << document.id << ", "s
              << "relevance = "s << document.relevance << ", "s
              << "rating = "s << document.rating << " }"s;
}

vector<string> SplitIntoWords(const string &text) {
    vector<string> words;
    string word;
    for (const char symbol : text) {
        if (symbol == ' ') {
            words.push_back(word);
            word = "";
        } else
            word += symbol;
    }

    words.push_back(word);
    return words;
}

// SearchServer class methods below

void SearchServer::SetStopWords(const string &text) {
    for (const string &word : SplitIntoWords(text))
        stop_words_.insert(word);
}

void SearchServer::AddDocument(int document_id, const string &document, DocumentStatus status,
                               const vector<int> &ratings) {
    if (const auto &error_message = CheckDocumentInput(document_id, document);
        error_message && !error_message->empty()) {
        throw std::invalid_argument(*error_message);
    }

    const vector<string> words = SplitDocumentIntoNoWords(document);
    const double inverse_word_count = 1. / words.size();

    for (const string &word : words)
        word_to_document_frequency_[word][document_id] += inverse_word_count;

    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
}

vector<Document> SearchServer::FindTopDocuments(const string &raw_query, DocumentStatus document_status) const {
    return FindTopDocuments(raw_query, [document_status](int document_id, DocumentStatus status, int rating) {
        return status == document_status;
    });
}

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(documents_.size());
}

SearchServer::WordsInDocumentInfo SearchServer::MatchDocument(const string &raw_query, int document_id) const {
    const Query query = ParseQuery(raw_query);
    vector<string> matched_words;

    for (const string &word : query.plus_words) {
        if (word_to_document_frequency_.count(word) == 0)
            continue;

        if (word_to_document_frequency_.at(word).count(document_id))
            matched_words.push_back(word);
    }

    for (const string &word : query.minus_words) {
        if (word_to_document_frequency_.count(word) == 0)
            continue;

        if (word_to_document_frequency_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }

    return {matched_words, documents_.at(document_id).status};
}

bool SearchServer::IsStopWord(const string &word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string &word) {
    // A valid word must not contain special characters in range [0, 31]
    return std::none_of(word.begin(), word.end(), [](char symbol) { return int(symbol) >= 0 && int(symbol) <= 31; });
}

vector<string> SearchServer::SplitDocumentIntoNoWords(const string &text) const {
    vector<string> words;

    for (const string &word : SplitIntoWords(text)) {
        if (!IsValidWord(word))
            throw std::invalid_argument("Invalid word in the document: "s + word);

        if (!IsStopWord(word))
            words.push_back(word);
    }

    return words;
}

int SearchServer::ComputeAverageRating(const vector<int> &ratings) {
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

SearchServer::Query SearchServer::ParseQuery(const string &query_text) const {
    Query query;
    for (const string &word : SplitIntoWords(query_text)) {
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
double SearchServer::ComputeWordInverseDocumentFrequency(const string &word) const {
    return log(GetDocumentCount() * 1. / word_to_document_frequency_.at(word).size());
}

}  // namespace sprint_1::server
