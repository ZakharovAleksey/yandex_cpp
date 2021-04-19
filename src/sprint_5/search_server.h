#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "document.h"
#include "string_processing.h"

namespace sprint_5::server {

using Word = std::string;
using DocumentId = int;

class SearchServer {
public:  // Public types
    using WordsInDocumentInfo = std::tuple<std::vector<std::string>, DocumentStatus>;

public:  // Constructors
    SearchServer() = default;

    template <class StringContainer>
    explicit SearchServer(const StringContainer &stop_words)
        : stop_words_(utils::MakeUniqueNonEmptyStrings(stop_words)) {}

    explicit SearchServer(const std::string &stop_words_text) : SearchServer(utils::SplitIntoWords(stop_words_text)) {}

public:  // Public methods
    template <class DocumentFilterFunction>
    std::vector<Document> FindTopDocuments(const std::string &raw_query, DocumentFilterFunction filter_function) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, filter_function);

        std::sort(matched_documents.begin(), matched_documents.end(), [](const Document &lhs, const Document &rhs) {
            return std::abs(lhs.relevance - rhs.relevance) < kEqualityThreshold ? lhs.rating > rhs.rating
                                                                                : lhs.relevance > rhs.relevance;
        });

        if (matched_documents.size() > static_cast<size_t>(kMaxDocumentsCount))
            matched_documents.resize(static_cast<size_t>(kMaxDocumentsCount));

        return matched_documents;
    }

    [[nodiscard]] std::vector<Document> FindTopDocuments(const std::string &raw_query,
                                                         DocumentStatus document_status = DocumentStatus::ACTUAL) const;

    [[nodiscard]] int GetDocumentCount() const;

    void SetStopWords(const std::string &text);

    void AddDocument(int document_id, const std::string &document, DocumentStatus status,
                     const std::vector<int> &ratings);

    [[nodiscard]] WordsInDocumentInfo MatchDocument(const std::string &raw_query, int document_id) const;

    [[nodiscard]] const std::map<Word, double> &GetWordFrequencies(DocumentId index) const;

    void RemoveDocument(DocumentId index);

    [[nodiscard]] std::set<int>::iterator begin();

    [[nodiscard]] std::set<int>::const_iterator begin() const;

    [[nodiscard]] std::set<int>::iterator end();

    [[nodiscard]] std::set<int>::const_iterator end() const;

private:  // Types
    struct DocumentData {
        int rating{0};
        DocumentStatus status;

        DocumentData() : status(DocumentStatus::IRRELEVANT) {}

        DocumentData(int rating, DocumentStatus status) : rating(rating), status(status) {}
    };

    struct QueryWord {
        std::string data;
        bool is_minus{false};
        bool is_stop{false};
    };

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

private:  // Constants
    static constexpr double kEqualityThreshold{1e-6};
    static constexpr int kMaxDocumentsCount{5};

private:  // Class methods
    template <class DocumentFilterFunction>
    std::vector<Document> FindAllDocuments(const Query &query, DocumentFilterFunction filter_function) const {
        std::map<int, double> document_to_relevance;
        for (const std::string &word : query.plus_words) {
            if (word_to_document_frequency_.count(word) == 0)
                continue;

            const double inverse_document_freq = ComputeWordInverseDocumentFrequency(word);
            for (const auto [document_id, term_freq] : word_to_document_frequency_.at(word)) {
                const auto &[rating, status] = documents_.at(document_id);
                if (filter_function(document_id, status, rating))
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }

        for (const std::string &word : query.minus_words) {
            if (word_to_document_frequency_.count(word) == 0)
                continue;

            for (const auto [document_id, _] : word_to_document_frequency_.at(word))
                document_to_relevance.erase(document_id);
        }

        std::vector<Document> matched_documents;
        matched_documents.reserve(document_to_relevance.size());
        for (const auto [document_id, relevance] : document_to_relevance)
            matched_documents.emplace_back(document_id, relevance, documents_.at(document_id).rating);

        return matched_documents;
    }

    static int ComputeAverageRating(const std::vector<int> &ratings);

    [[nodiscard]] bool IsStopWord(const std::string &word) const;

    [[nodiscard]] bool ParseQueryWord(std::string word, QueryWord &query_word) const;

    [[nodiscard]] std::vector<std::string> SplitDocumentIntoNoWords(const std::string &text) const;

    [[nodiscard]] Query ParseQuery(const std::string &query_text) const;

    [[nodiscard]] double ComputeWordInverseDocumentFrequency(const std::string &word) const;

    std::optional<std::string> CheckDocumentInput(int document_id, const std::string &document);

private:  // Class fields
    std::set<Word> stop_words_;
    std::map<Word, std::map<DocumentId, double>> word_to_document_frequency_;
    std::map<DocumentId, DocumentData> documents_;
    std::set<DocumentId> document_ids_;
    std::map<DocumentId, std::map<Word, double>> words_frequency_by_documents_;
};

void RemoveDuplicates(SearchServer &search_server);

}  // namespace sprint_5::server
