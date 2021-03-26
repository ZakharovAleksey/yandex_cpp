//
// Created by azakharov on 3/7/2021.
//

#include "server_test.h"

#include <cmath>

namespace unit_test {
    using namespace server;
    using namespace std::string_literals;

    // General variables, used for the big number of tests
    const std::string general_document_text = "cat dog puppy kitty";
    constexpr int general_document_id = 1;
    const std::vector<int> general_ratings = {1, 2, 3, 4};

    void TestExcludeStopWordsFromAddedDocumentContent() {
        {
            SearchServer server;
            server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

            const auto found_documents = server.FindTopDocuments("cat"s);
            ASSERT_EQUAL_HINT(found_documents.size(), 1,
                              "Expected number of documents WITHOUT stop words"s);

            ASSERT_EQUAL_HINT(found_documents[0].id, general_document_id,
                              "Expected document ID WITHOUT stop words"s);
        }

        {
            const std::string stop_word_from_document = "cat"s;

            SearchServer server;
            server.SetStopWords(stop_word_from_document);
            server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

            ASSERT_HINT(server.FindTopDocuments(stop_word_from_document).empty(),
                        "Skip documents if any word is a STOP word"s);
        }

        {
            const std::string stop_word = "none"s;

            SearchServer server;
            server.SetStopWords(stop_word);
            server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

            ASSERT_EQUAL_HINT(server.FindTopDocuments("cat"s).size(), 1,
                              "Expected number of documents found where STOP word IS NOT in the document"s);

            ASSERT_HINT(server.FindTopDocuments(stop_word).empty(),
                        "Expected number of documents found by STOP word query when it absent in document"s);
        }
    }

    void TestAddingDocumentsToTheServer() {
        {
            const std::string words_absent_in_general_document = "mom dad"s;

            SearchServer server;
            server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);
            server.AddDocument(general_document_id + 1, words_absent_in_general_document,
                               DocumentStatus::ACTUAL, general_ratings);

            ASSERT_EQUAL_HINT(server.GetDocumentCount(), 2, "All added documents should be found");

            for (const auto &word : SplitIntoWords(general_document_text)) {
                const auto found_documents = server.FindTopDocuments(word);

                ASSERT_EQUAL(found_documents.size(), 1);
                ASSERT_EQUAL_HINT(found_documents.at(0).id, general_document_id,
                                  "All words from the added document should be found"s);
            }
        }

        {
            SearchServer server;
            server.AddDocument(general_document_id, ""s, DocumentStatus::ACTUAL, general_ratings);
            ASSERT_EQUAL_HINT(server.GetDocumentCount(), 0, "Should not add document it it's empty"s);
        }

        {
            // TODO: Behaviour which I think we will fix further (plus all spaces behaviour)
            SearchServer server;
            server.AddDocument(general_document_id, "   "s, DocumentStatus::ACTUAL, general_ratings);
            ASSERT_EQUAL_HINT(server.GetDocumentCount(), 1,
                              "Should add document which consists only from spaces"s);
        }
    }


    void TestServerTakesIntoAccountMinusWords() {
        const std::string kQueryWithMinusWords = "cat -puppy"s;
        SearchServer server;

        server.AddDocument(1, "cat home"s, DocumentStatus::ACTUAL, general_ratings);
        server.AddDocument(2, "cat puppy home"s, DocumentStatus::ACTUAL, general_ratings);

        auto found_documents = server.FindTopDocuments(kQueryWithMinusWords);
        ASSERT_EQUAL_HINT(found_documents.size(), 1,
                          "Server returns ONLY the documents without minus words, which were in query"s);

        found_documents = server.FindTopDocuments("home puppy -puppy");
        ASSERT_EQUAL_HINT(found_documents.size(), 1,
                          "Server returns ONLY the documents without minus words, if query has a word and the same word as minus word"s);

        auto[matching_words, _] = server.MatchDocument(kQueryWithMinusWords, 1);
        ASSERT_HINT(!matching_words.empty(),
                    "Server matches the words if query has a minus word which absent in the document"s);

        matching_words = std::get<0>(server.MatchDocument(kQueryWithMinusWords, 2));
        ASSERT_HINT(matching_words.empty(),
                    "Server does not match any word for the document if it has at least one minus word"s);
    }

    void TestServerMatchWordsForTheDocument() {
        SearchServer server;
        server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

        auto[matching_words, _] = server.MatchDocument(""s, general_document_id);
        ASSERT_HINT(matching_words.empty(),
                    "Server does not match any word for the empty query"s);

        matching_words = std::get<0>(server.MatchDocument("none bug"s, general_document_id));
        ASSERT_HINT(matching_words.empty(),
                    "Server does not match any word if it is not in a document"s);

        matching_words = std::get<0>(server.MatchDocument("puppy cat"s, general_document_id));
        ASSERT_EQUAL_HINT(matching_words.size(), 2,
                          "Server matches all words from query in document"s);

        std::sort(matching_words.begin(), matching_words.end());
        ASSERT_EQUAL_HINT(matching_words[0], "cat"s,
                          "Server match the exact word from the query");
        ASSERT_EQUAL_HINT(matching_words[1], "puppy"s,
                          "Server match the exact word from the query");

        matching_words = std::get<0>(server.MatchDocument("cat dog -puppy", general_document_id));
        ASSERT_HINT(matching_words.empty(),
                    "Server does not match any word for the document if it has at least on minus word"s);

    }

    void TestFoundDocumentsSortingByRelevance() {
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

        ASSERT_EQUAL(server.GetDocumentCount(), 7);
        ASSERT_EQUAL(foundDocuments.size(), expectedDocumentIds.size());

        bool is_sorted_by_relevance = std::is_sorted(foundDocuments.begin(), foundDocuments.end(),
                                                     [](const Document &left, const Document &right) {
                                                         return left.relevance > right.relevance;
                                                     });

        ASSERT_HINT(is_sorted_by_relevance, "Server returns documents, sorted by relevance"s);
    }

    void TestServerFindNotMoreDocumentsThanExpected() {
        SearchServer server;

        server.AddDocument(1, "cat dog puppy kitty cat dog puppy kitty"s, DocumentStatus::ACTUAL, general_ratings);
        server.AddDocument(2, "cat dog puppy kitty"s, DocumentStatus::ACTUAL, general_ratings);
        server.AddDocument(3, "cat dog puppy kitty cat"s, DocumentStatus::ACTUAL, general_ratings);
        server.AddDocument(4, "cat dog puppy"s, DocumentStatus::ACTUAL, general_ratings);
        server.AddDocument(5, "cat dog"s, DocumentStatus::ACTUAL, general_ratings);
        server.AddDocument(6, "cat"s, DocumentStatus::ACTUAL, general_ratings);

        auto documents = server.FindTopDocuments(general_document_text);
        ASSERT_EQUAL_HINT(documents.size(), 5, "Server finds not more than 5 documents");
    }

    void TestDocumentRelevanceCalculation() {
        static const double kEqualityThreshold = 1e-3;
        static const int kDocumentsCount = 3;

        // Test on the data from the lectures
        SearchServer server;

        server.AddDocument(1, "white cat and fashion collar"s, DocumentStatus::ACTUAL, general_ratings);
        server.AddDocument(2, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, general_ratings);
        server.AddDocument(3, "well-groomed dog expressive eyes"s, DocumentStatus::ACTUAL, general_ratings);

        const auto found_documents = server.FindTopDocuments("fluffy well-groomed cat");
        ASSERT_EQUAL(found_documents.size(), 3);

        /// Map structure: "word in query" : { {"document index" : "word frequency in this document"}, ...}
        const std::map<std::string, std::map<int, double>> words_rating_data = {
                {"fluffy",       {{1, 0.},  {2, 0.5},  {3, 0.}}},
                {"well-groomed", {{1, 0.},  {2, 0.},   {3, 0.25}}},
                {"cat",          {{1, 0.2}, {2, 0.25}, {3, 0.}}}
        };

        std::map<int, double> actualRelevance;
        std::for_each(found_documents.begin(), found_documents.end(), [&actualRelevance](const Document &document) {
            actualRelevance[document.id] = document.relevance;
        });

        auto doubles_equal = [=](const double left, const double right) {
            return std::abs(left - right) < kEqualityThreshold;
        };

        auto calculate_relevance = [&words_rating_data](int document_id) {
            double relevance{0.};

            for (const auto&[word, index_to_frequency] : words_rating_data) {
                int documents_with_word_count = std::count_if(
                        index_to_frequency.begin(), index_to_frequency.end(),
                        [](const std::pair<int, double> pair) {
                            return pair.second > kEqualityThreshold;
                        });

                relevance += log(kDocumentsCount * 1. / documents_with_word_count) *
                             index_to_frequency.at(document_id);
            }
            return relevance;
        };

        for (const auto&[document_id, actual_relevance] : actualRelevance) {
            double expected_relevance = calculate_relevance(document_id);
            ASSERT_HINT(doubles_equal(expected_relevance, actual_relevance),
                        "Documents has expected relevance values"s);
        }

    }

    void TestDocumentRatingCalculation() {
        auto check_rating = [](const std::vector<int> &ratings, int expected_rating, const std::string &hint) {
            SearchServer server;

            server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, ratings);
            const auto found_documents = server.FindTopDocuments(general_document_text);

            ASSERT_EQUAL(found_documents.size(), 1);
            ASSERT_EQUAL_HINT(found_documents[0].rating, expected_rating, hint);
        };

        check_rating({}, 0,
                     "Server returns zero rating if input is empty"s);

        check_rating({1, 2, 4}, 2,
                     "Correct rating rounding for positive values with fractional part < 0.5"s);

        check_rating({1, 2, 5}, 2,
                     "Correct rating rounding for POSITIVE values with fractional part > 0.5"s);

        check_rating({-1, -2, -4}, -2,
                     "Correct rating rounding for NEGATIVE values with fractional part < 0.5"s);

        check_rating({-1, -2, -5}, -2,
                     "Correct rating rounding for NEGATIVE values with fractional part > 0.5"s);

        check_rating({-1, 2, 5}, 2,
                     "Correct rating in general case"s);

    }

    void TestFindDocumentsWithCustomDocumentStatusFilterFunction() {
        static const int kDocumentsCount = 3;
        SearchServer server;

        for (int document_id = general_document_id;
             document_id < general_document_id + kDocumentsCount; ++document_id) {
            server.AddDocument(document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);
        }

        auto custom_document_filter_function = [](int document_id, DocumentStatus status, int rating) {
            return document_id > general_document_id;
        };

        const auto found_documents = server.FindTopDocuments(general_document_text, custom_document_filter_function);
        ASSERT_EQUAL_HINT(found_documents.size(), kDocumentsCount - 1,
                          "Server found expected number of documents with custom document filter function"s);
    }

    void TestFindDocumentsWithDefaultDocumentStatusFilterFunction() {
        {
            static const int kDocumentStatusCount = 4;
            static const int kDocumentsCount = 9;
            std::map<DocumentStatus, std::vector<int>> expected_documents;

            SearchServer server;

            for (int document_id = 0; document_id < kDocumentsCount; ++document_id) {
                DocumentStatus status = static_cast<DocumentStatus>(document_id % kDocumentStatusCount);
                server.AddDocument(document_id, general_document_text, status, general_ratings);
                expected_documents[status].emplace_back(document_id);
            }


            auto check_status = [=](DocumentStatus status, const std::string &status_string,
                                    const std::vector<int> &expected_document_indexes) {
                const auto documents = server.FindTopDocuments(general_document_text, status);

                std::vector<int> actual_document_indexes;
                std::transform(documents.begin(), documents.end(), std::back_inserter(actual_document_indexes),
                               [](const Document &document) { return document.id; }
                );
                std::sort(actual_document_indexes.begin(), actual_document_indexes.end());

                ASSERT_EQUAL_HINT(documents.size(), expected_document_indexes.size(),
                                  "Server found correct number of documents with " + status_string + " status."s);

                ASSERT_HINT(std::equal(actual_document_indexes.begin(), actual_document_indexes.end(),
                                       expected_document_indexes.begin()),
                            "Server found correct document ID with " + status_string + " status."s);
            };

            check_status(DocumentStatus::ACTUAL, "ACTUAL", expected_documents[DocumentStatus::ACTUAL]);
            check_status(DocumentStatus::IRRELEVANT, "IRRELEVANT", expected_documents[DocumentStatus::IRRELEVANT]);
            check_status(DocumentStatus::BANNED, "BANNED", expected_documents[DocumentStatus::BANNED]);
            check_status(DocumentStatus::REMOVED, "REMOVED", expected_documents[DocumentStatus::REMOVED]);
        }

        {
            SearchServer server;
            server.AddDocument(general_document_id, general_document_text, DocumentStatus::ACTUAL, general_ratings);

            const auto found_documents = server.FindTopDocuments(general_document_text);
            ASSERT_EQUAL(found_documents.size(), 1);
            ASSERT_EQUAL_HINT(found_documents[0].id, general_document_id,
                              "Server found correct document with ACTUAL status and IMPLICIT ACTUAL function argument"s);

        }

    }

    void TestDocumentsCount() {
        static const int kAddedDocumentsCount = 4;

        SearchServer server;

        ASSERT_EQUAL_HINT(server.GetDocumentCount(), 0,
                          "Initially server does not have any documents"s);

        for (int document_id = 0; document_id < kAddedDocumentsCount; ++document_id) {
            std::string documentText = std::to_string(document_id) + " document text"s;
            server.AddDocument(general_document_id + document_id, documentText, DocumentStatus::ACTUAL,
                               general_ratings);
        }
        ASSERT_EQUAL_HINT(server.GetDocumentCount(), kAddedDocumentsCount,
                          "Server stored all added document"s);
    }

    void TestSearchServer() {
        RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
        RUN_TEST(TestAddingDocumentsToTheServer);
        RUN_TEST(TestServerTakesIntoAccountMinusWords);
        RUN_TEST(TestServerMatchWordsForTheDocument);
        RUN_TEST(TestServerFindNotMoreDocumentsThanExpected);
        RUN_TEST(TestFoundDocumentsSortingByRelevance);
        RUN_TEST(TestDocumentRelevanceCalculation);
        RUN_TEST(TestDocumentRatingCalculation);
        RUN_TEST(TestFindDocumentsWithCustomDocumentStatusFilterFunction);
        RUN_TEST(TestFindDocumentsWithDefaultDocumentStatusFilterFunction);
        RUN_TEST(TestDocumentsCount);
    }
}
