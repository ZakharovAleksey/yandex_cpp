//
// Created by azakharov on 4/19/2021.
//

#include <gtest/gtest.h>

#include <cmath>
#include <numeric>

#include "../src/sprint_8/paginator.h"

using namespace sprint_8::server::utils;
using namespace std::literals;

TEST(PaginatorClass, PaginatorSplitWhenContainerSizeIsDivisibleOnPageSize) {
    const int size = 12;
    const int page_size = 3;
    const std::vector<std::vector<int>> expected_pages = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
    auto expected_page = expected_pages.begin();

    std::vector<int> values(size);
    std::iota(values.begin(), values.end(), 1);

    auto paginator = Paginate(values, page_size);
    EXPECT_EQ(paginator.size(), size / page_size) << "Paginator should split container on expected number of pages"s;

    for (auto& page : paginator) {
        std::vector<int> actual_page{page.start, page.end};
        EXPECT_EQ(actual_page, *expected_page++)
            << "Paginator should correctly split container on pages with expected content"s;
    }
}

TEST(PaginatorClass, PaginatorSplitWhenContainerSizeIsNotDivisibleOnPageSize) {
    const int size = 10;
    const int page_size = 3;
    const std::vector<std::vector<int>> expected_pages = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10}};
    auto expected_page = expected_pages.begin();

    std::vector<int> values(size);
    std::iota(values.begin(), values.end(), 1);

    auto paginator = Paginate(values, page_size);
    EXPECT_EQ(paginator.size(), static_cast<size_t>(ceil(size * 1. / page_size)))
        << "Paginator should split container on expected number of pages"s;

    for (auto& page : paginator) {
        std::vector<int> actual_page{page.start, page.end};
        EXPECT_EQ(actual_page, *expected_page++)
            << "Paginator should correctly split container on pages with expected content"s;
    }
}

TEST(PaginatorClass, PaginatorSplitContainerWithSizeLessThanPageSize) {
    const int size = 2;
    const int page_size = 3;
    const std::vector<std::vector<int>> expected_pages = {{1, 2}};
    auto expected_page = expected_pages.begin();

    std::vector<int> values(size);
    std::iota(values.begin(), values.end(), 1);

    auto paginator = Paginate(values, page_size);
    EXPECT_EQ(paginator.size(), 1u) << "Paginator should have one page if container size less that page size"s;

    for (auto& page : paginator) {
        std::vector<int> actual_page{page.start, page.end};
        EXPECT_EQ(actual_page, *expected_page++)
            << "Paginator should correctly split container on pages with expected content"s;
    }
}

TEST(PaginatorClass, PaginatorSplitEmptyContainer) {
    const int page_size = 3;
    std::vector<int> values;

    auto paginator = Paginate(values, page_size);
    EXPECT_EQ(paginator.size(), 0u) << "Paginator should not split on pages empty container"s;
}