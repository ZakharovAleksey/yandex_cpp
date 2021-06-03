//
// Created by azakharov on 6/2/2021.
//

#pragma once

#include <algorithm>
#include <execution>
#include <future>
#include <mutex>
#include <type_traits>
#include <vector>

namespace sprint_8::server {

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
        bucket.map_.erase(key);
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

}  // namespace sprint_8::server