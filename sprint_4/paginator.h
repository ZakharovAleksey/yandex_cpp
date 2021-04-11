//
// Created by azakharov on 4/4/2021.
//

#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

namespace sprint_4::server::utils {

template <class Iterator>
struct IteratorsRange {
    Iterator start;
    Iterator end;

    explicit IteratorsRange(Iterator range_start, Iterator range_end) : start(range_start), end(range_end) {}
};

template <class Iterator>
std::ostream& operator<<(std::ostream& os, const IteratorsRange<Iterator>& range) {
    for (auto iter = range.start; iter != range.end; ++iter)
        os << *iter;
    return os;
}

template <class Iterator>
class Paginator {
    public:  // Constructor
    Paginator() = default;

    public:  // Methods
    void Init(Iterator begin, Iterator end, int page_size) {
        int elements_left{std::distance(begin, end)};
        int elements_fit_page{0};

        while (elements_left > 0) {
            elements_fit_page = std::min(page_size, elements_left);
            pages_.emplace_back(begin, begin + elements_fit_page);

            begin += elements_fit_page;
            elements_left -= elements_fit_page;
        }
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

    size_t size() const {
        return pages_.size();
    }

    private:  // fields
    std::vector<IteratorsRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& container, int page_size) {
    Paginator<decltype(container.begin())> paginator;
    paginator.Init(container.begin(), container.end(), page_size);

    return paginator;
}

}  // namespace sprint_4::server::utils