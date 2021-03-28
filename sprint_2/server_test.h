//
// Created by azakharov on 3/7/2021.
//

#pragma once

#include "../sprint_1/server.h"
#include "testing_framework.h"

namespace unit_test {
void TestExcludeStopWordsFromAddedDocumentContent();

void TestAddingDocumentsToTheServer();

void TestServerTakesIntoAccountMinusWords();

void TestServerMatchWordsForTheDocument();

void TestServerFindNotMoreDocumentsThanExpected();

void TestFoundDocumentsSortingByRelevance();

void TestDocumentRelevanceCalculation();

void TestDocumentRatingCalculation();

void TestFindDocumentsWithCustomDocumentStatusFilterFunction();

void TestFindDocumentsWithDefaultDocumentStatusFilterFunction();

void TestDocumentsCount();

void TestSearchServer();
}  // namespace unit_test
