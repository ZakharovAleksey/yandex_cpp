//
// Created by azakharov on 4/19/2021.
//

#include <gtest/gtest.h>

#include "../src/sprint_8/request_queue.h"

using namespace sprint_8::server;
using namespace sprint_8::server::utils;
using namespace std::literals;

TEST(RequestQueueClass, TestRequestQueueWithoutDocuments) {
    const int empty_request_count{10};

    SearchServer server;
    RequestQueue queue(server);

    for (int request_id = 0; request_id < empty_request_count; ++request_id)
        queue.AddFindRequest("empty request"s);

    EXPECT_EQ(queue.GetNoResultRequests(), empty_request_count)
        << "Number of empty responses should be equal to number of requests if there is no documents on the server"s;
}

TEST(RequestQueueClass, TestRequestQueueReturnsEmptyDocumnetCount) {
    SearchServer server;
    RequestQueue queue(server);

    server.AddDocument(1, "cat dog"s, DocumentStatus::ACTUAL, {1, 2, 3});

    queue.AddFindRequest("cat boy"s);
    queue.AddFindRequest("dog girl"s);
    EXPECT_EQ(queue.GetNoResultRequests(), 0)
        << "Query should detect number of empty responses correctly, if there were no one"s;

    queue.AddFindRequest("girl boy"s);
    EXPECT_EQ(queue.GetNoResultRequests(), 1)
        << "Query should detect number of empty responses correctly, if there was one"s;
}

TEST(RequestQueueClass, TestRequestQueueUpdateOnNewDay) {
    const int requests_count = 1439;  //> From RequestQueue class - 1

    SearchServer server("and in at"s);
    RequestQueue queue(server);

    server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

    for (int request_id = 0; request_id < requests_count; ++request_id)
        queue.AddFindRequest("empty request"s);
    ASSERT_EQ(queue.GetNoResultRequests(), requests_count);

    queue.AddFindRequest("curly dog"s);
    ASSERT_EQ(queue.GetNoResultRequests(), requests_count)
        << "Queue should correctly handle the last event in the day"s;

    queue.AddFindRequest("big collar"s);
    ASSERT_EQ(queue.GetNoResultRequests(), requests_count - 1)
        << "Queue should remove the first query when new day has come"s;

    queue.AddFindRequest("sparrow"s);
    ASSERT_EQ(queue.GetNoResultRequests(), requests_count - 2)
        << "Queue should correctly calculate number of empty responses on the day change"s;
}
