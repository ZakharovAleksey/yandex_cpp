//
// Created by azakharov on 4/4/2021.
//

#pragma once

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
    for (auto iter = range.start; iter != range.end; iter = std::next(iter))
        os << *iter;
    return os;
}

template <class Iterator>
class Paginator {
   public:  // Constructor
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        size_t elements_on_page{1u};

        for (auto iterator = begin; iterator != end; iterator = std::next(iterator), ++elements_on_page) {
            if (elements_on_page == 1u)
                pages_.emplace_back(iterator, iterator);

            if (elements_on_page == page_size) {
                pages_.back().end = std::next(iterator);
                elements_on_page = 0u;
            }
        }

        if (!pages_.empty())
            pages_.back().end = end;
    }

   public:  // Methods
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
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

}  // namespace sprint_4::server::utils