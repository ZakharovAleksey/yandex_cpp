#include <iostream>

#include "sprint_1/server.h"

void PrintDocument(const server::Document &document)
{
    using namespace std;

    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}

int main()
{
    using namespace std;
    using namespace server;

    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});


    cout << "ACTUAL by default:"s << endl;
    for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s))
        PrintDocument(document);


    cout << "BANNED:"s << endl;
    for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED))
        PrintDocument(document);


    cout << "Even ids:"s << endl;
    auto even_document_ids = [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; };
    for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s, even_document_ids))
        PrintDocument(document);

    return 0;
}