//
// Created by azakharov on 2/19/2021.
//

#ifndef YP_CPP1_SERVER_H
#define YP_CPP1_SERVER_H


#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

namespace Server_NS
{
    constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;

    std::string ReadLine();

    int ReadLineWithNumber();

    std::vector<std::string> SplitIntoWords(const std::string &text);

    struct Document
    {
        int id;
        double relevance;
        int rating;
    };

    enum class DocumentStatus
    {
        ACTUAL,
        IRRELEVANT,
        BANNED,
        REMOVED,
    };

    class SearchServer
    {
    public:
        void SetStopWords(const std::string &text);

        void AddDocument(int document_id, const std::string &document, DocumentStatus status, const std::vector<int> &ratings);

        template <class DocFilterFunc>
        std::vector<Document> FindTopDocuments(const std::string &raw_query, DocFilterFunc filter_func) const
        {
            const Query query = ParseQuery(raw_query);
            auto matched_documents = FindAllDocuments(query, filter_func);

            std::sort(matched_documents.begin(), matched_documents.end(),
                      [](const Document &lhs, const Document &rhs) {
                          if (std::abs(lhs.relevance - rhs.relevance) < 1e-6)
                              return lhs.rating > rhs.rating;
                          else
                              return lhs.relevance > rhs.relevance;
                      });
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
            {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return matched_documents;
        }

        std::vector<Document> FindTopDocuments(const std::string &raw_query) const;

        std::vector<Document> FindTopDocuments(const std::string &raw_query, DocumentStatus doc_status) const;

        int GetDocumentCount() const;

        std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string &raw_query, int document_id) const;

    private:
        struct DocumentData
        {
            int rating;
            DocumentStatus status;
        };

        std::set<std::string> stop_words_;
        std::map<std::string, std::map<int, double>> word_to_document_freqs_;
        std::map<int, DocumentData> documents_;

        bool IsStopWord(const std::string &word) const;

        std::vector<std::string> SplitIntoWordsNoStop(const std::string &text) const;

        static int ComputeAverageRating(const std::vector<int> &ratings);

        struct QueryWord
        {
            std::string data;
            bool is_minus;
            bool is_stop;
        };

        QueryWord ParseQueryWord(std::string text) const;

        struct Query
        {
            std::set<std::string> plus_words;
            std::set<std::string> minus_words;
        };

        Query ParseQuery(const std::string &text) const;

        // Existence required
        double ComputeWordInverseDocumentFreq(const std::string &word) const;

        template <class DocFilterFunc>
        std::vector<Document> FindAllDocuments(const Query &query, DocFilterFunc filter_func) const
        {
            std::map<int, double> document_to_relevance;
            for (const std::string &word : query.plus_words)
            {
                if (word_to_document_freqs_.count(word) == 0)
                {
                    continue;
                }
                const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
                {
                    const auto &[rating, status] = documents_.at(document_id);
                    if (filter_func(document_id, status, rating))
                    {
                        document_to_relevance[document_id] += term_freq * inverse_document_freq;
                    }
                }
            }

            for (const std::string &word : query.minus_words)
            {
                if (word_to_document_freqs_.count(word) == 0)
                {
                    continue;
                }
                for (const auto [document_id, _] : word_to_document_freqs_.at(word))
                {
                    document_to_relevance.erase(document_id);
                }
            }

            std::vector<Document> matched_documents;
            for (const auto [document_id, relevance] : document_to_relevance)
            {
                matched_documents.push_back({document_id,
                                             relevance,
                                             documents_.at(document_id).rating});
            }
            return matched_documents;
        }
    };
}


#endif //YP_CPP1_SERVER_H
