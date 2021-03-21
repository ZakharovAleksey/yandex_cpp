//
// Created by azakharov on 2/19/2021.
//

#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

namespace server
{
    struct Document
    {
        int id {0};
        double relevance {0.};
        int rating {0};
    };

    enum class DocumentStatus
    {
        ACTUAL,
        IRRELEVANT,
        BANNED,
        REMOVED,
    };

    std::string ReadLine();

    int ReadLineWithNumber();

    std::vector<std::string> SplitIntoWords(const std::string &text);

    class SearchServer
    {
    public: // Public types
        using WordsInDocumentInfo = std::tuple<std::vector<std::string>, DocumentStatus>;

    public: // Public methods
        template <class DocumentFilterFunction>
        std::vector<Document> FindTopDocuments(const std::string &raw_query,
                                               DocumentFilterFunction filter_function) const
        {
            const Query query = ParseQuery(raw_query);
            auto matched_documents = FindAllDocuments(query, filter_function);

            std::sort(matched_documents.begin(), matched_documents.end(),
                      [](const Document &lhs, const Document &rhs) {
                          if (std::abs(lhs.relevance - rhs.relevance) < kEqualityThreshold)
                              return lhs.rating > rhs.rating;
                          else
                              return lhs.relevance > rhs.relevance;
                      });

            if (matched_documents.size() > static_cast<size_t>(kMaxDocumentsCount))
                matched_documents.resize(static_cast<size_t>(kMaxDocumentsCount));

            return matched_documents;
        }

        std::vector<Document> FindTopDocuments(const std::string &raw_query,
                                               DocumentStatus document_status = DocumentStatus::ACTUAL) const;

        void SetStopWords(const std::string &text);

        void AddDocument(int document_id, const std::string &document,
                         DocumentStatus status, const std::vector<int> &ratings);

        int GetDocumentCount() const;

        WordsInDocumentInfo MatchDocument(const std::string &raw_query, int document_id) const;

    private: // Types
        struct DocumentData
        {
            int rating {0};
            DocumentStatus status;
        };
        struct QueryWord
        {
            std::string data;
            bool is_minus {false};
            bool is_stop {false};
        };
        struct Query
        {
            std::set<std::string> plus_words;
            std::set<std::string> minus_words;
        };

    private: // Constants
        static constexpr double kEqualityThreshold {1e-6};
        static constexpr int kMaxDocumentsCount {5};

    private: // Class methods
        template <class DocumentFilterFunction>
        std::vector<Document> FindAllDocuments(const Query &query, DocumentFilterFunction filter_function) const
        {
            std::map<int, double> document_to_relevance;
            for (const std::string &word : query.plus_words)
            {
                if (word_to_document_frequency_.count(word) == 0)
                    continue;

                const double inverse_document_freq = ComputeWordInverseDocumentFrequency(word);
                for (const auto [document_id, term_freq] : word_to_document_frequency_.at(word))
                {
                    const auto &[rating, status] = documents_.at(document_id);
                    if (filter_function(document_id, status, rating))
                        document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }

            for (const std::string &word : query.minus_words)
            {
                if (word_to_document_frequency_.count(word) == 0)
                    continue;

                for (const auto [document_id, _] : word_to_document_frequency_.at(word))
                    document_to_relevance.erase(document_id);
            }

            std::vector<Document> matched_documents;
            for (const auto [document_id, relevance] : document_to_relevance)
            {
                matched_documents.push_back({
                    document_id, relevance, documents_.at(document_id).rating
                });
            }

            return matched_documents;
        }

        static int ComputeAverageRating(const std::vector<int> &ratings);

        bool IsStopWord(const std::string &word) const;

        std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const;

        QueryWord ParseQueryWord(std::string text) const;

        Query ParseQuery(const std::string &text) const;

        double ComputeWordInverseDocumentFrequency(const std::string &word) const;

    private: // Class fields
        std::set<std::string> stop_words_;
        std::map<std::string, std::map<int, double>> word_to_document_frequency_;
        std::map<int, DocumentData> documents_;
    };
}
