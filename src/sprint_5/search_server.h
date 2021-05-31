#pragma once

#include <algorithm>
#include <execution>
#include <map>
#include <optional>
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
    using WordsInDocumentInfo = std::tuple<std::vector<std::string_view>, DocumentStatus>;

public:  // Constructors
    SearchServer() = default;

    template <class StringContainer>
    explicit SearchServer(const StringContainer &stop_words)
        : stop_words_(utils::MakeUniqueNonEmptyStrings(stop_words)) {}

    explicit SearchServer(const std::string &stop_words_text) : SearchServer(utils::SplitIntoWords(stop_words_text)) {}

    explicit SearchServer(std::string_view stop_words_text) : SearchServer(utils::SplitIntoWords(stop_words_text)) {}

public:  // Public methods
    template <class DocumentFilterFunction>
    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentFilterFunction filter_function) const {
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

    [[nodiscard]] std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                                         DocumentStatus document_status = DocumentStatus::ACTUAL) const;

    [[nodiscard]] int GetDocumentCount() const;

    void SetStopWords(std::string_view text);

    void AddDocument(int document_id, std::string_view document, DocumentStatus status,
                     const std::vector<int> &ratings);

    [[nodiscard]] WordsInDocumentInfo MatchDocument(std::string_view raw_query, int document_id) const;

    template <class ExecutionPolicyName>
    [[nodiscard]] WordsInDocumentInfo MatchDocument(ExecutionPolicyName policy, std::string_view raw_query,
                                                    int document_id) const {
        using namespace std::execution;
        std::vector<std::string_view> matched_words;

        if (!(std::is_same<ExecutionPolicyName, parallel_policy>::value ||
              std::is_same<ExecutionPolicyName, sequenced_policy>::value))
            return {matched_words, documents_.at(document_id).status};

        const Query query = ParseQuery(policy, raw_query);

        const auto &plus_words = query.plus_words;
        std::for_each(
            policy, plus_words.begin(), plus_words.end(), [this, document_id, &matched_words](std::string_view word) {
                const auto word_position = word_to_document_frequency_.find(word);
                if (word_position != word_to_document_frequency_.end() && word_position->second.count(document_id))
                    matched_words.push_back(word_position->first);
            });

        // clang-format off
        const auto &minus_words = query.minus_words;
        bool has_minus_words =
            std::transform_reduce(minus_words.begin(), minus_words.end(), 0, std::plus<>(),
                [this, document_id](std::string_view word) {
                                      const auto word_position = word_to_document_frequency_.find(word);
                                      return (word_position != word_to_document_frequency_.end() &&
                                              word_position->second.count(document_id)) ? 1 : 0; }
                                  ) > 0;
        // clang-format on

        if (has_minus_words)
            matched_words.clear();

        return {matched_words, documents_.at(document_id).status};
    }

    [[nodiscard]] const std::map<std::string, double> &GetWordFrequencies(DocumentId index) const;

    void RemoveDocument(DocumentId index);

    template <class ExecutionPolicyType>
    void RemoveDocument(ExecutionPolicyType policy, DocumentId index) {
        using namespace std::execution;
        // Currently we support only two policies: seq & par
        if (!(std::is_same<ExecutionPolicyType, sequenced_policy>::value ||
              std::is_same<ExecutionPolicyType, parallel_policy>::value))
            return;

        auto document_position = document_ids_.find(index);
        if (document_position == document_ids_.end())
            return;
        document_ids_.erase(document_position);

        auto &words_in_document = words_frequency_by_documents_.at(index);
        std::for_each(policy, words_in_document.begin(), words_in_document.end(),
                      [this, index](const auto &pair) { word_to_document_frequency_.at(pair.first).erase(index); });

        documents_.erase(index);
        words_frequency_by_documents_.erase(index);
    }

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
        std::string_view data;
        bool is_minus{false};
        bool is_stop{false};
    };

    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

private:  // Constants
    static constexpr double kEqualityThreshold{1e-6};
    static constexpr int kMaxDocumentsCount{5};

private:  // Class methods
    template <class DocumentFilterFunction>
    std::vector<Document> FindAllDocuments(const Query &query, DocumentFilterFunction filter_function) const {
        std::map<int, double> document_to_relevance;
        for (std::string_view word : query.plus_words) {
            const auto word_position = word_to_document_frequency_.find(word);
            if (word_position == word_to_document_frequency_.cend())
                continue;

            const double inverse_document_freq = ComputeWordInverseDocumentFrequency(word);
            for (const auto [document_id, term_freq] : word_position->second) {
                const auto &[rating, status] = documents_.at(document_id);
                if (filter_function(document_id, status, rating))
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }

        for (std::string_view word : query.minus_words) {
            const auto word_position = word_to_document_frequency_.find(word);
            if (word_position == word_to_document_frequency_.cend())
                continue;

            for (const auto [document_id, _] : word_position->second)
                document_to_relevance.erase(document_id);
        }

        std::vector<Document> matched_documents;
        matched_documents.reserve(document_to_relevance.size());
        for (const auto [document_id, relevance] : document_to_relevance)
            matched_documents.emplace_back(document_id, relevance, documents_.at(document_id).rating);

        return matched_documents;
    }

    static int ComputeAverageRating(const std::vector<int> &ratings);

    [[nodiscard]] bool IsStopWord(std::string_view word) const;

    [[nodiscard]] bool ParseQueryWord(std::string_view word, QueryWord &query_word) const;

    [[nodiscard]] std::vector<std::string_view> SplitDocumentIntoNoWords(std::string_view text) const;

    [[nodiscard]] Query ParseQuery(std::string_view query_text) const;

    template <class ExecutionPolicyType>
    [[nodiscard]] Query ParseQuery(ExecutionPolicyType policy, std::string_view query_text) const {
        using namespace std::literals;
        using namespace std::execution;
        using namespace sprint_5::server::utils;

        // Currently we support only two policies: seq & par
        if (!(std::is_same<ExecutionPolicyType, sequenced_policy>::value ||
              std::is_same<ExecutionPolicyType, parallel_policy>::value))
            return {};

        Query query;
        std::vector<std::string_view> words = SplitIntoWords(query_text);

        // Convert all words to the query-words
        std::vector<std::pair<bool, QueryWord>> query_words(words.size());
        std::transform(words.begin(), words.end(), query_words.begin(), [this](std::string_view word) {
            QueryWord query_word;
            return std::make_pair(ParseQueryWord(word, query_word), query_word);
        });

        // Check that at least one word is invalid
        auto invalid_word_position = std::find_if(std::execution::par, query_words.cbegin(), query_words.cend(),
                                                  [](const std::pair<bool, QueryWord> &pair) { return !pair.first; });
        if (invalid_word_position != query_words.cend())
            throw std::invalid_argument("Invalid word in the query: "s +
                                        std::string(invalid_word_position->second.data));

        // Move words from the storage to the appropriate category <plus / minus>
        for (auto [_, query_word] : query_words) {
            if (!query_word.is_stop) {
                if (query_word.is_minus)
                    query.minus_words.insert(query_word.data);
                else
                    query.plus_words.insert(query_word.data);
            }
        }

        return query;
    }

    [[nodiscard]] double ComputeWordInverseDocumentFrequency(std::string_view word) const;

    std::optional<std::string> CheckDocumentInput(int document_id, std::string_view document);

private:  // Class fields
    std::set<Word, std::less<>> stop_words_;
    std::map<Word, std::map<DocumentId, double>, std::less<>> word_to_document_frequency_;
    std::map<DocumentId, DocumentData> documents_;
    std::set<DocumentId> document_ids_;
    std::map<DocumentId, std::map<Word, double>> words_frequency_by_documents_;
};

void RemoveDuplicates(SearchServer &search_server);

}  // namespace sprint_5::server
