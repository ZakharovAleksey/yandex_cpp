//
// Created by azakharov on 4/19/2021.
//

#include <gtest/gtest.h>

#include <cmath>
#include <execution>

#include "../src/sprint_5/search_server.h"

using namespace sprint_5::server;
using namespace sprint_5::server::utils;
using namespace std::literals;

// General variables, used for the big number of tests
const std::string general_document_text = "cat dog puppy kitty"s;
constexpr int general_document_id = 1;
const std::vector<int> general_ratings = {1, 2, 3, 4};

TEST(SearchServerClass, ExcludeStopWordsWhichAreNotInQuery) {
    SearchServer server;
    server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

    const auto found_documents = server.FindTopDocuments("cat"s);
    EXPECT_EQ(found_documents.size(), 1) << "Expected number of documents WITHOUT stop words"s;
    EXPECT_EQ(found_documents[0].id, general_document_id) << "Expected document ID WITHOUT stop words"s;
}

TEST(SearchServerClass, ExcludeStopWordsWhichAreInQuery) {
    const std::string stop_word_from_document = "cat"s;

    SearchServer server(stop_word_from_document);
    server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

    EXPECT_TRUE(server.FindTopDocuments(stop_word_from_document).empty())
        << "Skip documents if any word is a STOP word"s;
}

TEST(SearchServerClass, ExcludeStopWordsWhichAreNotInDocument) {
    const std::string stop_word = "none"s;

    SearchServer server(stop_word);
    server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

    EXPECT_EQ(server.FindTopDocuments("cat"s).size(), 1)
        << "Expected number of documents found where STOP word IS NOT in the document"s;

    EXPECT_TRUE(server.FindTopDocuments(stop_word).empty())
        << "Expected number of documents found by STOP word query when it absent in document"s;
}

TEST(SearchServerClass, TestAddingDocumentsToTheServer) {
    const std::string words_absent_in_general_document = "mom dad"s;

    SearchServer server;
    server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(general_document_id + 1, words_absent_in_general_document, DocumentStatus::ACTUAL,
                       general_ratings);

    EXPECT_EQ(server.GetDocumentCount(), 2) << "All added documents should be found";

    for (std::string_view word : SplitIntoWords(general_document_text)) {
        const auto found_documents = server.FindTopDocuments(std::string(word));

        EXPECT_EQ(found_documents.size(), 1);
        EXPECT_EQ(found_documents.at(0).id, general_document_id)
            << "All words from the added document should be found"s;
    }
}

TEST(SearchServerClass, TestAddingEmptyDocumentToTheServer) {
    SearchServer server;
    EXPECT_THROW(server.AddDocument(general_document_id, ""s, DocumentStatus::ACTUAL, general_ratings),
                 std::invalid_argument)
        << "Server should throw if document is empty"s;
}

TEST(SearchServerClass, TestAddingDocumentsWithExistedIdsToTheServer) {
    SearchServer server;
    server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);
    EXPECT_THROW(server.AddDocument(general_document_id, "new text"s, DocumentStatus::ACTUAL, general_ratings),
                 std::invalid_argument)
        << "Should throw if document index is already in data base"s;
}

TEST(SearchServerClass, TestAddingDocumentsWithNegativeIdsToTheServer) {
    SearchServer server;
    EXPECT_THROW(server.AddDocument(-1, general_document_text, DocumentStatus::ACTUAL, general_ratings),
                 std::invalid_argument)
        << "Should throw if document index is negative"s;
}

TEST(SearchServerClass, TestAddingDocumentFromSpacesOnly) {
    // TODO: Behaviour which I think we will fix further (plus all
    SearchServer server;
    server.AddDocument(general_document_id, "   "s, DocumentStatus::ACTUAL, general_ratings);
    EXPECT_EQ(server.GetDocumentCount(), 1) << "Should add document which consists only from spaces"s;
}

TEST(SearchServerClass, TestServerTakesIntoAccountMinusWords) {
    const std::string kQueryWithMinusWords = "cat -puppy"s;
    SearchServer server;

    server.AddDocument(1, "cat home"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(2, "cat puppy home"s, DocumentStatus::ACTUAL, general_ratings);

    auto found_documents = server.FindTopDocuments(kQueryWithMinusWords);
    EXPECT_EQ(found_documents.size(), 1)
        << "Server returns ONLY the documents without minus words, which were in query"s;

    found_documents = server.FindTopDocuments("home puppy -puppy");
    EXPECT_EQ(found_documents.size(), 1)
        << "Server returns ONLY the documents without minus words, if query has a word and the same word as minus word"s;

    auto [matching_words, _] = server.MatchDocument(kQueryWithMinusWords, 1);
    EXPECT_TRUE(!matching_words.empty())
        << "Server matches the words if query has a minus word which absent in the document"s;

    std::tie(matching_words, _) = server.MatchDocument(kQueryWithMinusWords, 2);
    EXPECT_TRUE(matching_words.empty())
        << "Server does not match any word for the document if it has at least one minus word"s;
}

TEST(SearchServerClass, TestServerMatchWordsForTheDocument) {
    int max_special_symbol_index{31};
    SearchServer server;

    server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

    EXPECT_THROW(auto matching_result = server.MatchDocument(""s, general_document_id), std::invalid_argument)
        << "Server should throw if matching query is empty"s;

    std::string query;
    for (int symbol_index = 0; symbol_index < max_special_symbol_index; ++symbol_index) {
        query = "cat"s + static_cast<char>(symbol_index) + " dog"s;
        EXPECT_THROW(auto matching_result = server.MatchDocument(query, general_document_id), std::invalid_argument)
            << "Server should throw if at least one special symbol is detected"s;
    }

    auto [matching_words, _] = server.MatchDocument("none bug"s, general_document_id);
    EXPECT_TRUE(matching_words.empty()) << "Server does not match any word if it is not in a document"s;

    std::tie(matching_words, _) = server.MatchDocument("puppy cat"s, general_document_id);
    EXPECT_EQ(matching_words.size(), 2) << "Server matches all words from query in document"s;

    std::sort(matching_words.begin(), matching_words.end());
    EXPECT_EQ(matching_words[0], "cat"s) << "Server match the exact word from the query"s;
    EXPECT_EQ(matching_words[1], "puppy"s) << "Server match the exact word from the query"s;

    std::tie(matching_words, _) = server.MatchDocument("cat dog -puppy", general_document_id);
    EXPECT_TRUE(matching_words.empty())
        << "Server does not match any word for the document if it has at least on minus word"s;
}

TEST(SearchServerClass, TestFoundDocumentsSortingByRelevance) {
    SearchServer server;

    server.AddDocument(1, "one"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(2, "one two"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(3, "one two three"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(4, "one two three four"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(5, "one two three four five"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(6, "one two three four five six"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(7, "one two three four five six seven"s, DocumentStatus::ACTUAL, general_ratings);

    const auto foundDocuments = server.FindTopDocuments("one three five seven"s);
    /// Server returns NOT more than 5 matched documents by default
    const std::vector<int> expectedDocumentIds = {7, 5, 6, 3, 4};

    EXPECT_EQ(server.GetDocumentCount(), 7);
    EXPECT_EQ(foundDocuments.size(), expectedDocumentIds.size());

    bool is_sorted_by_relevance =
        std::is_sorted(foundDocuments.begin(), foundDocuments.end(),
                       [](const Document& left, const Document& right) { return left.relevance > right.relevance; });

    EXPECT_TRUE(is_sorted_by_relevance) << "Server returns documents, sorted by relevance"s;
}

TEST(SearchServerClass, TestServerFindNotMoreDocumentsThanExpected) {
    SearchServer server;

    server.AddDocument(1, "cat dog puppy kitty cat dog puppy kitty"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(2, "cat dog puppy kitty"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(3, "cat dog puppy kitty cat"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(4, "cat dog puppy"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(5, "cat dog"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(6, "cat"s, DocumentStatus::ACTUAL, general_ratings);

    auto documents = server.FindTopDocuments(general_document_text);
    EXPECT_EQ(documents.size(), 5) << "Server finds not more than 5 documents"s;
}

TEST(SearchServerClass, TestDocumentRelevanceCalculation) {
    static const double kEqualityThreshold = 1e-3;
    const int documents_count = 3;

    // Test on the data from the lectures
    SearchServer server;

    server.AddDocument(1, "white cat and fashion collar"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(2, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, general_ratings);
    server.AddDocument(3, "well-groomed dog expressive eyes"s, DocumentStatus::ACTUAL, general_ratings);

    const auto found_documents = server.FindTopDocuments("fluffy well-groomed cat");
    EXPECT_EQ(found_documents.size(), 3);

    /// Map structure: "word in query" : { {"document index" : "word
    /// frequency in this document"}, ...}
    const std::map<std::string, std::map<int, double>> words_rating_data = {
        {"fluffy", {{1, 0.}, {2, 0.5}, {3, 0.}}},
        {"well-groomed", {{1, 0.}, {2, 0.}, {3, 0.25}}},
        {"cat", {{1, 0.2}, {2, 0.25}, {3, 0.}}}};

    std::map<int, double> actualRelevance;
    std::for_each(found_documents.begin(), found_documents.end(),
                  [&actualRelevance](const Document& document) { actualRelevance[document.id] = document.relevance; });

    auto calculate_relevance = [&](int document_id) {
        double relevance{0.};

        for (const auto& [word, index_to_frequency] : words_rating_data) {
            int documents_with_word_count =
                std::count_if(index_to_frequency.begin(), index_to_frequency.end(),
                              [](const std::pair<int, double> pair) { return pair.second > kEqualityThreshold; });

            relevance += log(documents_count * 1. / documents_with_word_count) * index_to_frequency.at(document_id);
        }
        return relevance;
    };

    for (const auto& [document_id, actual_relevance] : actualRelevance) {
        double expected_relevance = calculate_relevance(document_id);
        EXPECT_NEAR(expected_relevance, actual_relevance, kEqualityThreshold)
            << "Documents has expected relevance values"s;
    }
}

TEST(SearchServerClass, TestDocumentRatingCalculation) {
    auto check_rating = [](const std::vector<int>& ratings, int expected_rating, const std::string& hint) {
        SearchServer server;

        server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, ratings);
        const auto found_documents = server.FindTopDocuments(general_document_text);

        EXPECT_EQ(found_documents.size(), 1);
        EXPECT_EQ(found_documents[0].rating, expected_rating) << hint;
    };

    check_rating({}, 0, "Server returns zero rating if input is empty"s);

    check_rating({1, 2, 4}, 2, "Correct rating rounding for positive values with fractional part < 0.5"s);

    check_rating({1, 2, 5}, 2, "Correct rating rounding for POSITIVE values with fractional part > 0.5"s);

    check_rating({-1, -2, -4}, -2, "Correct rating rounding for NEGATIVE values with fractional part < 0.5"s);

    check_rating({-1, -2, -5}, -2, "Correct rating rounding for NEGATIVE values with fractional part > 0.5"s);

    check_rating({-1, 2, 5}, 2, "Correct rating in general case"s);
}

TEST(SearchServerClass, TestFindDocumentsWithCustomDocumentStatusFilterFunction) {
    static const int documents_count = 3;
    SearchServer server;

    for (int document_id = general_document_id; document_id < general_document_id + documents_count; ++document_id) {
        server.AddDocument(document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);
    }

    auto custom_document_filter_function = [](int document_id, DocumentStatus status, int rating) {
        return document_id > general_document_id;
    };

    const auto found_documents = server.FindTopDocuments(general_document_text, custom_document_filter_function);
    EXPECT_EQ(found_documents.size(), documents_count - 1)
        << "Server found expected number of documents with custom document filter function"s;
}

TEST(SearchServerClass, TestFindDocumentsWithDefaultDocumentStatusFilterFunctionExplicit) {
    const int document_status_count = 4;
    const int documents_count = 9;
    std::map<DocumentStatus, std::vector<int>> expected_documents;

    SearchServer server;

    for (int document_id = 0; document_id < documents_count; ++document_id) {
        auto status = static_cast<DocumentStatus>(document_id % document_status_count);
        server.AddDocument(document_id, general_document_text, status, general_ratings);
        expected_documents[status].emplace_back(document_id);
    }

    auto check_status = [=](DocumentStatus status, const std::string& status_string,
                            const std::vector<int>& expected_document_indexes) {
        const auto documents = server.FindTopDocuments(general_document_text, status);

        std::vector<int> actual_document_indexes;
        std::transform(documents.begin(), documents.end(), std::back_inserter(actual_document_indexes),
                       [](const Document& document) { return document.id; });
        std::sort(actual_document_indexes.begin(), actual_document_indexes.end());

        EXPECT_EQ(documents.size(), expected_document_indexes.size())
            << "Server found correct number of documents with " + status_string + " status."s;

        EXPECT_TRUE(std::equal(actual_document_indexes.begin(), actual_document_indexes.end(),
                               expected_document_indexes.begin()))
            << "Server found correct document ID with " + status_string + " status."s;
    };

    check_status(DocumentStatus::ACTUAL, "ACTUAL", expected_documents[DocumentStatus::ACTUAL]);
    check_status(DocumentStatus::IRRELEVANT, "IRRELEVANT", expected_documents[DocumentStatus::IRRELEVANT]);
    check_status(DocumentStatus::BANNED, "BANNED", expected_documents[DocumentStatus::BANNED]);
    check_status(DocumentStatus::REMOVED, "REMOVED", expected_documents[DocumentStatus::REMOVED]);
}

TEST(SearchServerClass, TestFindDocumentsWithDefaultDocumentStatusFilterFunctionImplicit) {
    SearchServer server;
    server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

    const auto found_documents = server.FindTopDocuments(general_document_text);
    EXPECT_EQ(found_documents.size(), 1);
    EXPECT_EQ(found_documents[0].id, general_document_id)
        << "Server found correct document with ACTUAL status and IMPLICIT ACTUAL function argument"s;
}

TEST(SearchServerClass, TestDocumentsCount) {
    const int added_documents_count = 4;

    SearchServer server;

    EXPECT_EQ(server.GetDocumentCount(), 0) << "Initially server does not have any documents"s;

    for (int document_id = 0; document_id < added_documents_count; ++document_id) {
        std::string documentText = std::to_string(document_id) + " document text"s;
        server.AddDocument(general_document_id + document_id, documentText, DocumentStatus::ACTUAL, general_ratings);
    }
    EXPECT_EQ(server.GetDocumentCount(), added_documents_count) << "Server stored all added document"s;
}

TEST(SearchServerClass, TestRemoveDocumentWithDiffererntExecutionPlocies) {
    SearchServer search_server("and with"s);
    std::vector<std::string> input_documents = {
        "funny pet and nasty rat"s,      "funny pet with curly hair"s, "funny pet and not very nasty rat"s,
        "pet with rat and rat and rat"s, "nasty rat with curly hair"s,
    };
    int document_index = 0;
    int current_documents_count = input_documents.size();

    for (const std::string& text : input_documents)
        search_server.AddDocument(++document_index, text, DocumentStatus::ACTUAL, {1, 2});

    const std::string query = "curly and funny"s;

    auto report = [&search_server, &query](int expected_documents_count, const std::string& execution_policy = ""s) {
        EXPECT_EQ(search_server.GetDocumentCount(), expected_documents_count)
            << "Number of document should decrease on 1 after RemoveDocument() method call. "s + execution_policy;

        EXPECT_EQ(search_server.FindTopDocuments(query).size(), expected_documents_count - 1)
            << "Method RemoveDocument() should remove document with SPECIFIED index. "s + execution_policy;
    };

    report(current_documents_count--, "Single-thread policy"s);
    search_server.RemoveDocument(5);

    report(current_documents_count--, "Single-thread policy with std::execution::seq"s);
    search_server.RemoveDocument(std::execution::seq, 1);

    report(current_documents_count--, "Multy-thread policy with std::execution::par"s);
    search_server.RemoveDocument(std::execution::par, 2);

    report(current_documents_count--);
}

TEST(SearchServerClass, TestMatchDocumentsWithDifferentExecutionPolices) {
    SearchServer search_server("and with"s);
    std::vector<std::string> input_documents = {
        "funny pet and nasty rat"s,      "funny pet with curly hair"s, "funny pet and not very nasty rat"s,
        "pet with rat and rat and rat"s, "nasty rat with curly hair"s,
    };
    int document_index = 0;
    int current_documents_count = input_documents.size();

    for (const std::string& text : input_documents)
        search_server.AddDocument(++document_index, text, DocumentStatus::ACTUAL, {1, 2});

    const std::string query = "curly and funny -not"s;

    {
        const auto [words, status] = search_server.MatchDocument(query, 1);
        EXPECT_EQ(words.size(), 1) << "Unexpected behaviour in method MatchDocument() with Single-thread policy"s;
    }
    {
        const auto [words, status] = search_server.MatchDocument(std::execution::seq, query, 2);
        EXPECT_EQ(words.size(), 2)
            << "Unexpected behaviour in method MatchDocument() with Single-thread policy (execution::seq)"s;
    }
    {
        const auto [words, status] = search_server.MatchDocument(std::execution::par, query, 3);
        EXPECT_EQ(words.size(), 0)
            << "Unexpected behaviour in method MatchDocument() with Multi-thread policy (execution::par)"s;
    }
}