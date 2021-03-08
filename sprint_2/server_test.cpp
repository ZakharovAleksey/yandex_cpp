//
// Created by azakharov on 3/7/2021.
//

#include "server_test.h"

namespace unit_test {
    using namespace server;
    using namespace std::string_literals;

    // General variables, used for the big number of tests
    const std::string kGeneralDocumentText = "cat dog puppy kitty";
    constexpr int kGeneralDocumentId = 1;
    const std::vector<int> kGeneralRatings = {1, 2, 3, 4};

    void TestExcludeStopWordsFromAddedDocumentContent() {

        {
            SearchServer server;
            server.AddDocument(kGeneralDocumentId, kGeneralDocumentText, DocumentStatus::ACTUAL, kGeneralRatings);

            const auto found_documents = server.FindTopDocuments("cat"s);
            ASSERT_EQUAL_HINT(found_documents.size(), 1,
                              "Expected number of documents WITHOUT stop words"s);

            ASSERT_EQUAL_HINT(found_documents[0].id, kGeneralDocumentId,
                              "Expected document ID WITHOUT stop words"s);
        }

        {
            const std::string stop_word_from_document = "cat"s;

            SearchServer server;
            server.SetStopWords(stop_word_from_document);
            server.AddDocument(kGeneralDocumentId, kGeneralDocumentText, DocumentStatus::ACTUAL, kGeneralRatings);

            ASSERT_HINT(server.FindTopDocuments(stop_word_from_document).empty(),
                        "Skip documents if any word is a STOP word"s);
        }

        {
            const std::string stop_word = "none"s;

            SearchServer server;
            server.SetStopWords(stop_word);
            server.AddDocument(kGeneralDocumentId, kGeneralDocumentText, DocumentStatus::ACTUAL, kGeneralRatings);

            ASSERT_EQUAL_HINT(server.FindTopDocuments("cat"s).size(), 1,
                              "Expected number of documents found where STOP word IS NOT in the document"s);

            ASSERT_HINT(server.FindTopDocuments(stop_word).empty(),
                        "Expected number of documents found by STOP word query when it absent in document"s);
        }
    }

    void TestAddDocument() {
        {
            const std::string words_absent_in_general_document = "mom dad"s;

            SearchServer server;
            server.AddDocument(kGeneralDocumentId, kGeneralDocumentText, DocumentStatus::ACTUAL, kGeneralRatings);
            server.AddDocument(kGeneralDocumentId + 1, words_absent_in_general_document,
                               DocumentStatus::ACTUAL, kGeneralRatings);

            ASSERT_EQUAL_HINT(server.GetDocumentCount(), 2, "All added documents should be found");

            for (const auto &word : SplitIntoWords(kGeneralDocumentText)) {
                const auto found_documents = server.FindTopDocuments(word);

                ASSERT_EQUAL(found_documents.size(), 1);
                ASSERT_EQUAL_HINT(found_documents.at(0).id, kGeneralDocumentId,
                                  "All words from the added document should be found"s);
            }
        }

        {
            SearchServer server;
            server.AddDocument(kGeneralDocumentId, ""s, DocumentStatus::ACTUAL, kGeneralRatings);
            ASSERT_EQUAL_HINT(server.GetDocumentCount(), 0, "Should not add document it it's empty"s);
        }

        {
            // TODO: Behaviour which I think we will fix further (plus all spaces behaviour)
            SearchServer server;
            server.AddDocument(kGeneralDocumentId, "   "s, DocumentStatus::ACTUAL, kGeneralRatings);
            ASSERT_EQUAL_HINT(server.GetDocumentCount(), 1,
                              "Should add document which consists only from spaces"s);
        }
    }


    void TestMinusWords() {
        const std::string kQueryWithMinusWords = "cat -puppy"s;
        SearchServer server;

        server.AddDocument(1, "cat home"s, DocumentStatus::ACTUAL, kGeneralRatings);
        server.AddDocument(2, "cat puppy home"s, DocumentStatus::ACTUAL, kGeneralRatings);

        auto found_documents = server.FindTopDocuments(kQueryWithMinusWords);
        ASSERT_EQUAL_HINT(found_documents.size(), 1,
                          "Server returns ONLY the documents without minus words, which were in query"s);

        found_documents = server.FindTopDocuments("home puppy -puppy");
        ASSERT_EQUAL_HINT(found_documents.size(), 1,
                          "Server returns ONLY the documents without minus words, if query has a word and the same word as minus word"s);

        auto matching_words = std::get<0>(server.MatchDocument(kQueryWithMinusWords, 1));
        ASSERT_HINT(!matching_words.empty(),
                    "Server matches the words if query has a minus word which absent in the document"s);

        matching_words = std::get<0>(server.MatchDocument(kQueryWithMinusWords, 2));
        ASSERT_HINT(matching_words.empty(),
                    "Server does not match any word for the document if it has at least one minus word"s);
    }

    void TestMatchingWords() {
        SearchServer server;
        server.AddDocument(kGeneralDocumentId, kGeneralDocumentText, DocumentStatus::ACTUAL, kGeneralRatings);

        auto matching_words = std::get<0>(server.MatchDocument(""s, kGeneralDocumentId));
        ASSERT_HINT(matching_words.empty(),
                    "Server does not match any word for the empty query"s);

        matching_words = std::get<0>(server.MatchDocument("none bug"s, kGeneralDocumentId));
        ASSERT_HINT(matching_words.empty(),
                    "Server does not match any word if it is not in a document"s);

        matching_words = std::get<0>(server.MatchDocument("puppy cat"s, kGeneralDocumentId));
        ASSERT_EQUAL_HINT(matching_words.size(), 2,
                          "Server matches all words from query in document"s);

        std::sort(matching_words.begin(), matching_words.end());
        ASSERT_EQUAL_HINT(matching_words[0], "cat"s,
                          "Server match the exact word from the query");
        ASSERT_EQUAL_HINT(matching_words[1], "puppy"s,
                          "Server match the exact word from the query");

        matching_words = std::get<0>(server.MatchDocument("cat dog -puppy", kGeneralDocumentId));
        ASSERT_HINT(matching_words.empty(),
                    "Server does not match any word for the document if it has at least on minus word"s);

    }

    void TestRelevance() {
        {
            SearchServer server;

            server.AddDocument(1, "cat dog puppy kitty cat dog puppy kitty"s, DocumentStatus::ACTUAL, kGeneralRatings);
            server.AddDocument(2, "cat dog puppy kitty"s, DocumentStatus::ACTUAL, kGeneralRatings);
            server.AddDocument(3, "cat dog puppy kitty cat"s, DocumentStatus::ACTUAL, kGeneralRatings);
            server.AddDocument(4, "cat dog puppy"s, DocumentStatus::ACTUAL, kGeneralRatings);
            server.AddDocument(5, "cat dog"s, DocumentStatus::ACTUAL, kGeneralRatings);
            server.AddDocument(6, "cat"s, DocumentStatus::ACTUAL, kGeneralRatings);

            auto documents = server.FindTopDocuments(kGeneralDocumentText);
            ASSERT_EQUAL_HINT(documents.size(), 5, "Server finds not more than 5 documents");
        }

        {
            // Test on the data from the lectures
            SearchServer server;

            server.AddDocument(1, "white cat and fashion collar"s, DocumentStatus::ACTUAL, kGeneralRatings);
            server.AddDocument(2, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, kGeneralRatings);
            server.AddDocument(3, "well-groomed dog expressive eyes"s, DocumentStatus::ACTUAL, kGeneralRatings);

            const auto found_documents = server.FindTopDocuments("fluffy well-groomed cat");
            ASSERT_EQUAL(found_documents.size(), 3);


            const std::vector<int> expectedRelevanceOrder = {2, 3, 1};
            std::vector<int> actualRelevanceOrder;
            std::transform(found_documents.begin(), found_documents.end(), std::back_inserter(actualRelevanceOrder),
                           [](const Document &document) {
                               return document.id;
                           });

            ASSERT_HINT(std::equal(actualRelevanceOrder.begin(), actualRelevanceOrder.end(),
                                   expectedRelevanceOrder.begin()),
                        "Documents has expected relevance order"s);


            const std::vector<double> expectedRelevance = {0.1014, 0.6507, 0.2748};
            std::vector<int> actualRelevance;
            std::transform(found_documents.begin(), found_documents.end(), std::back_inserter(actualRelevance),
                           [](const Document &document) {
                               return document.relevance;
                           });

            auto positive_doubles_equal = [](const double left, const double right) {
                const double kEqualityThreshold = 1e-3;
                return std::abs(left - right) < kEqualityThreshold;
            };

            ASSERT_HINT(std::equal(actualRelevanceOrder.begin(), actualRelevanceOrder.end(),
                                   expectedRelevanceOrder.begin(), positive_doubles_equal),
                        "Documents has expected relevance values"s);

        }
    }

    void TestRating() {

        auto check_rating = [](const std::vector<int> &ratings, int expected_rating, const std::string &hint) {
            SearchServer server;

            server.AddDocument(kGeneralDocumentId, kGeneralDocumentText, DocumentStatus::ACTUAL, ratings);
            const auto found_documents = server.FindTopDocuments(kGeneralDocumentText);

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

    void TestStatus() {
        {
            const int kDocumentStatusCount = 4;
            const int kDocumentsCount = 9;
            std::map<DocumentStatus, std::vector<int>> expected_documents;

            SearchServer server;

            for (int document_id = 0; document_id < kDocumentsCount; ++document_id) {
                DocumentStatus status = static_cast<DocumentStatus>(document_id % kDocumentStatusCount);
                server.AddDocument(document_id, kGeneralDocumentText, status, kGeneralRatings);
                expected_documents[status].emplace_back(document_id);
            }


            auto check_status = [=](DocumentStatus status, const std::string &status_string,
                                    const std::vector<int> &expected_document_indexes) {
                const auto documents = server.FindTopDocuments(kGeneralDocumentText, status);

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
            server.AddDocument(kGeneralDocumentId, kGeneralDocumentText, DocumentStatus::ACTUAL, kGeneralRatings);

            const auto found_documents = server.FindTopDocuments(kGeneralDocumentText);
            ASSERT_EQUAL(found_documents.size(), 1);
            ASSERT_EQUAL_HINT(found_documents[0].id, kGeneralDocumentId,
                              "Server found correct document with ACTUAL status and IMPLICIT ACTUAL function argument"s);

        }

        {
            const int kDocumentsCount = 3;

            SearchServer server;
            for (int document_id = kGeneralDocumentId;
                 document_id < kGeneralDocumentId + kDocumentsCount; ++document_id) {
                server.AddDocument(document_id, kGeneralDocumentText, DocumentStatus::ACTUAL, kGeneralRatings);
            }

            auto custom_document_filter_function = [](int document_id, DocumentStatus status, int rating) {
                return document_id > kGeneralDocumentId;
            };

            const auto found_documents = server.FindTopDocuments(kGeneralDocumentText, custom_document_filter_function);
            ASSERT_EQUAL_HINT(found_documents.size(), kDocumentsCount - 1,
                              "Server found expected number of documents with custom document filter function"s);
        }
    }

    void TestSearchServer() {
        RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
        RUN_TEST(TestAddDocument);
        RUN_TEST(TestMinusWords);
        RUN_TEST(TestMatchingWords);
        RUN_TEST(TestRelevance);
        RUN_TEST(TestRating);
        RUN_TEST(TestStatus);
    }
}
