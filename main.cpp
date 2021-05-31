#include <ctime>
#include <execution>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include "src/sprint_5/log_duration.h"
#include "src/sprint_5/search_server.h"

using namespace std;
using namespace sprint_5::server::utils;
using namespace sprint_5::server;

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server,
                                                  const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> responses(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), responses.begin(),
                   [&search_server](const std::string& query) { return search_server.FindTopDocuments(query); });
    return responses;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
    auto responses = ProcessQueries(search_server, queries);
    size_t total_documents_count = std::transform_reduce(responses.begin(), responses.end(), 0, std::plus<size_t>(),
                                                         [](const auto& response) { return response.size(); });
    std::vector<Document> result(total_documents_count);
    std::for_each(std::execution::par, responses.begin(), responses.end(), [&result](auto response) {
        for (auto document : response)
            result.push_back(std::move(document));
    });

    return result;
}

string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    static const std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
    static std::default_random_engine rng(std::time(nullptr));
    static std::uniform_int_distribution<std::size_t> distribution(0, alphabet.size() - 1);

    while (word.size() < length)
        word += alphabet[distribution(rng)];

    return word;
}

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int max_word_count) {
    const int word_count = uniform_int_distribution(1, max_word_count)(generator);
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count,
                               int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}

template <typename ExecutionPolicy>
void Test(string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
    LOG_DURATION(mark, std::cout);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(policy, id);
    }
    cout << search_server.GetDocumentCount() << endl;
}

#define TEST(mode) Test(#mode, search_server, execution::mode)

 int main() {
     mt19937 generator;

     const auto dictionary = GenerateDictionary(generator, 10000, 25);
     const auto documents = GenerateQueries(generator, dictionary, 10'000, 100);

     SearchServer search_server(dictionary[0]);
     for (size_t i = 0; i < documents.size(); ++i) {
         search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
     }

     TEST(seq);
     TEST(par);
 }
