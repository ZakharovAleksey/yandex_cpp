//
// Created by azakharov on 3/7/2021.
//

#ifndef YP_CPP1_SEARCH_SERVER_TEST_H
#define YP_CPP1_SEARCH_SERVER_TEST_H

#include "../sprint_1/server.h"
#include "testing_framework.h"

namespace unit_test {
    void TestExcludeStopWordsFromAddedDocumentContent();

    void TestAddDocument();

    void TestMinusWords();

    void TestMatchingWords();

    void TestRelevance();

    void TestRating();

    void TestStatus();

    void TestSearchServer();
}


#endif //YP_CPP1_SEARCH_SERVER_TEST_H
