//
// Created by azakharov on 2/19/2021.
//

#include "server.h"

#include <cmath>
#include <iostream>

namespace server
{
    using std::string;
    using std::vector;

    string ReadLine()
    {
        string out_str;
        getline(std::cin, out_str);

        return out_str;
    }

    int ReadLineWithNumber()
    {
        int number;
        std::cin >> number;
        ReadLine();

        return number;
    }

    vector<string> SplitIntoWords(const string &text)
    {
        vector<string> words;
        string word;
        for (const char symbol : text)
        {
            if (symbol == ' ')
            {
                words.push_back(word);
                word = "";
            }
            else
                word += symbol;
        }

        words.push_back(word);
        return words;
    }

    // SearchServer class methods below

    void SearchServer::SetStopWords(const string &text)
    {
        for (const string &word : SplitIntoWords(text))
            stop_words_.insert(word);
    }

    void SearchServer::AddDocument(int document_id, const string &document,
                                   DocumentStatus status, const vector<int> &ratings)
    {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inverse_word_count = 1.0 / words.size();

        for (const string &word : words)
            word_to_document_frequency_[word][document_id] += inverse_word_count;

        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    }

    vector<Document> SearchServer::FindTopDocuments(const string &raw_query, DocumentStatus document_status) const
    {
        return FindTopDocuments(raw_query, [document_status](int document_id, DocumentStatus status, int rating) {
            return status == document_status;
        });
    }

    int SearchServer::GetDocumentCount() const
    {
        return static_cast<int>(documents_.size());
    }

    SearchServer::WordsInDocumentInfo SearchServer::MatchDocument(const string &raw_query, int document_id) const
    {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;

        for (const string &word : query.plus_words)
        {
            if (word_to_document_frequency_.count(word) == 0)
                continue;

            if (word_to_document_frequency_.at(word).count(document_id))
                matched_words.push_back(word);
        }

        for (const string &word : query.minus_words)
        {
            if (word_to_document_frequency_.count(word) == 0)
                continue;

            if (word_to_document_frequency_.at(word).count(document_id))
            {
                matched_words.clear();
                break;
            }
        }

        return {matched_words, documents_.at(document_id).status};
    }

    bool SearchServer::IsStopWord(const string &word) const
    {
        return stop_words_.count(word) > 0;
    }

    vector<string> SearchServer::SplitIntoWordsNoStop(const string &text) const
    {
        vector<string> words;

        for (const string &word : SplitIntoWords(text))
            if (!IsStopWord(word))
                words.push_back(word);

        return words;
    }

    int SearchServer::ComputeAverageRating(const vector<int> &ratings)
    {
        int rating_sum = 0;
        for (const int rating : ratings)
            rating_sum += rating;

        return rating_sum / static_cast<int>(ratings.size());
    }

    SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const
    {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-')
        {
            is_minus = true;
            text = text.substr(1);
        }

        return {text, is_minus, IsStopWord(text)};
    }

    SearchServer::Query SearchServer::ParseQuery(const string &text) const
    {
        Query query;
        for (const string &word : SplitIntoWords(text))
        {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop)
            {
                if (query_word.is_minus)
                    query.minus_words.insert(query_word.data);
                else
                    query.plus_words.insert(query_word.data);
            }
        }

        return query;
    }

    // Existence required
    double SearchServer::ComputeWordInverseDocumentFrequency(const string &word) const
    {
        return log(GetDocumentCount() * 1.0 / word_to_document_frequency_.at(word).size());
    }
}
