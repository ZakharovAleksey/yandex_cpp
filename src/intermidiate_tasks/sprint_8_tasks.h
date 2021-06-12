//
// Created by azakharov on 6/3/2021.
//

#pagma once

#include <cctype>
#include <cstring>
#include <future>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string_view>
#include <vector>

using namespace std::literals;

namespace intermediate {

/*
 * Multi-threading implementation of word frequencies calculation
 */
struct Stats {
    std::map<std::string, int> word_frequences;

    void operator+=(const Stats& other) {
        for (const auto& [word, count] : other.word_frequences)
            word_frequences[word] += count;
    }
};

using KeyWords = std::set<std::string, std::less<>>;

Stats SplitIntoWords(const KeyWords& key_words, std::string text) {
    Stats result;
    auto word_begin = text.begin();
    std::string current_word;

    while (word_begin != text.end()) {
        auto word_end = std::find_if(word_begin, text.end(), [](char symbol) { return symbol == ' '; });
        current_word = {word_begin, word_end};
        if (ispunct(current_word.back()))
            current_word.pop_back();

        if (!current_word.empty() && key_words.count(current_word) > 0)
            ++result.word_frequences[std::string(current_word)];

        word_begin = word_end == text.end() ? word_end : std::next(word_end);
    }
    return result;
}

constexpr int MAX_STRINGS_COUNT{50'000};
constexpr int MAX_WORDS_IN_LINE{20};
constexpr int MAX_WORD_LENGTH{6};

Stats ExploreKeyWords(const KeyWords& key_words, std::istream& input) {
    std::vector<std::future<Stats>> stats_futures;
    stats_futures.reserve(MAX_STRINGS_COUNT);

    std::vector<std::string> lines;
    lines.reserve(MAX_STRINGS_COUNT);

    std::string line;
    line.reserve(MAX_WORD_LENGTH * MAX_WORDS_IN_LINE);
    while (std::getline(input, line))
        lines.emplace_back(std::move(line));

    for (auto& line : lines)
        stats_futures.emplace_back(std::async(std::launch::async, SplitIntoWords, std::ref(key_words), line));

    Stats result;
    for (auto& future : stats_futures)
        result += future.get();
    return result;
}

/*
 * Make std::for_each multi-threading analog for containers without random access iterators
 */

template <typename ForwardRangeContainer>
auto CalculateBucketsBoundariesForContainer(ForwardRangeContainer& container, int buckets_count, int bucket_size) {
    using InteratorType = typename ForwardRangeContainer::iterator;

    std::vector<std::pair<InteratorType, InteratorType>> buckets_boundaries;
    buckets_boundaries.reserve(buckets_count);

    auto begin = container.begin();
    auto end = container.begin();
    for (int page_id = 0; page_id < buckets_count; ++page_id) {
        end = begin;
        for (int element_id = 0; element_id < bucket_size; ++element_id) {
            end = std::next(end);
            if (end == container.end())
                break;
        }
        buckets_boundaries.emplace_back(std::make_pair(begin, end));
        begin = end;
    }

    return buckets_boundaries;
}

template <class ExecutionPolicy, typename ForwardRangeContainer, typename Function>
void ForEach(ExecutionPolicy policy, ForwardRangeContainer& container, Function function) {
    if (container.empty())
        return;

    using InteratorType = typename ForwardRangeContainer::iterator;
    using IteratorCategory = typename std::iterator_traits<InteratorType>::iterator_category;

    // Calculates number of buckets & bucket size, depending on processors count
    // !!! Trick with Container of non-random access iterator - use .size() to get bucket boundaries in one O(n) loop
    auto calculate_buckets_sizes = [](int container_size) {
        const auto processor_count = static_cast<int>(std::thread::hardware_concurrency());
        int bucket_size = container_size > processor_count ? container_size / processor_count : container_size;
        int buckets_count = container_size / bucket_size;
        buckets_count += (container_size % bucket_size == 0) ? 0 : 1;

        return std::make_pair(buckets_count, bucket_size);
    };

    // If ExecutionPolicy is parallel and Container with non-random access iterator - split function between threads
    if constexpr (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::parallel_policy> &&
                  !std::is_same<IteratorCategory, std::random_access_iterator_tag>::value) {
        const auto [buckets_count, bucket_size] = calculate_buckets_sizes(container.size());

        std::vector<std::future<void>> futures_vector;
        futures_vector.reserve(buckets_count);
        auto buckets_boundaries = CalculateBucketsBoundariesForContainer(container, buckets_count, bucket_size);

        for (const auto& p : buckets_boundaries)
            futures_vector.emplace_back(std::async([&] { std::for_each(policy, p.first, p.second, function); }));

        for (auto& future : futures_vector)
            future.get();
    } else
        std::for_each(policy, container.begin(), container.end(), function);
}

template <typename ForwardRange, typename Function>
void ForEach(ForwardRange& range, Function function) {
    std::for_each(range.begin(), range.end(), function);
}

/*
 * Make multi-treading analog of std::copy_if()
 */
constexpr int MAX_SIZE = 1'000'000;

template <typename Container, typename Predicate>
vector<typename Container::value_type> CopyIfUnordered(const Container& container, Predicate predicate) {
    vector<typename Container::value_type> result(container.size());
    std::atomic<int> id{0};
    std::for_each(std::execution::par, container.begin(), container.end(),
                  [&result, &predicate, &id](const auto& value) {
                      if (predicate(value))
                          result[id++] = value;
                  });

    return {std::make_move_iterator(result.begin()), std::make_move_iterator(result.begin() + id)};
}

/*
 * Make thread-safe std::map<k,v> analog
 */

template <typename Key, typename Value>
class ConcurrentMap {
public:  // Types
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");
    using MapType = std::map<Key, Value>;

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    struct ThreadSafeMap {
        std::map<Key, Value> map_;
        std::mutex mutex_;
    };

public:  // Constructors
    explicit ConcurrentMap(size_t bucket_count) : bucket_count_(bucket_count), buckets_(bucket_count_) {}

public:  // Methods
    Access operator[](const Key& key) {
        auto& bucket = buckets_[getBucketId(key)];
        return {std::lock_guard(bucket.mutex_), bucket.map_[key]};
    }

    void erase(const Key& key) {
        auto& bucket = buckets_[getBucketId(key)];
        std::lock_guard guard(bucket.mutex_);
        bucket.map_.Erase(key);
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        MapType result;
        for (auto& bucket : buckets_) {
            for (auto& [key, value] : bucket.map_) {
                std::lock_guard<std::mutex> guard(bucket.mutex_);
                result[key] = value;
            }
        }
        return result;
    }

private:  // Methods
    size_t getBucketId(const Key& key) const {
        return static_cast<uint64_t>(key) % bucket_count_;
    }

private:  // Fields
    size_t bucket_count_{0u};
    std::vector<ThreadSafeMap> buckets_;
};

}  // namespace intermediate