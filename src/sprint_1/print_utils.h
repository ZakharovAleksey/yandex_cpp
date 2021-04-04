//
// Created by azakharov on 4/2/2021.
//

#pragma once

#include <iostream>
#include <map>
#include <set>
#include <vector>

namespace sprint_1::server::utils {
using namespace std::literals;

template <class Iterator>
std::ostream& PrintRange(std::ostream& os, Iterator begin, Iterator end) {
    size_t index{0u};
    for (auto iterator = begin; iterator != end; iterator = std::next(iterator)) {
        if (index++ != 0u)
            os << " "s << *iterator;
        os << *iterator;
    }
    return os;
}

template <class Type>
std::ostream& operator<<(std::ostream& os, const std::vector<Type>& vector) {
    os << "[ "s;
    PrintRange(os, vector.begin(), vector.end());
    os << " ]"s;

    return os;
}

template <class Type>
std::ostream& operator<<(std::ostream& os, const std::set<Type>& vector) {
    os << "{"s;
    PrintRange(os, vector.begin(), vector.end());
    os << " }"s;

    return os;
}

template <class Key, class Value>
std::ostream& operator<<(std::ostream& os, const std::pair<Key, Value>& p) {
    os << "("s << p.first << ", "s << p.second << ")"s;
    return os;
}

template <class Key, class Value>
std::ostream& operator<<(std::ostream& os, const std::map<Key, Value>& m) {
    os << "< "s;
    if (!m.empty())
        PrintRange(os, m.begin(), m.end());
    os << " >"s;
    return os;
}

}  // namespace sprint_1::server::utils