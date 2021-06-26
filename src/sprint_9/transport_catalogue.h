#pragma once

#include <deque>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

using StringViewPair = std::pair<std::string_view, std::string_view>;
using DistancesToStops = std::vector<std::pair<std::string_view, int>>;

namespace catalog {

enum class RouteType { CIRCLE, TWO_DIRECTIONAL };

struct Bus {
    std::string number;
    RouteType type;
    std::vector<std::string_view> stop_names;
    std::set<std::string_view> unique_stops;

    [[nodiscard]] size_t GetStopsCount() const;
};

struct Stop {
    std::string name;
    Coordinates point;
};

struct BusStatistics {
    std::string_view number;
    size_t stops_count{0u};
    size_t unique_stops_count{0u};
    uint64_t rout_length{0};
    double curvature{0.};
};

class TransportCatalogue {
public:  // Constructors
    TransportCatalogue() = default;

public:  // Methods
    void AddStop(Stop stop);
    void AddBus(Bus bus);
    void AddDistancesBetweenStops(std::string_view stop_from, const DistancesToStops& distances);

    [[nodiscard]] std::optional<BusStatistics> GetBusStatistics(std::string_view bus_number) const;
    [[nodiscard]] std::optional<std::set<std::string_view>> GetBusesPassingThroughTheStop(
        std::string_view stop_name) const;

private:  // Types
    struct StringViewPairHash {
        size_t operator()(const StringViewPair& pair) const {
            return hasher(pair.first) + prime_number * hasher(pair.second);
        }

    private:
        std::hash<std::string_view> hasher;
        static const size_t prime_number{31};
    };

private:  // Methods
    uint64_t CalculateRouteLength(const Bus* bus_info) const;
    double CalculateGeographicLength(const Bus* bus_info) const;

private:  // Fields
    std::deque<Stop> stops_storage_;
    std::unordered_map<std::string_view, const Stop*> stops_;

    std::deque<Bus> buses_storage_;
    std::unordered_map<std::string_view, const Bus*> buses_;

    std::unordered_map<std::string_view, std::set<std::string_view>> buses_through_stop_;
    std::unordered_map<StringViewPair, int, StringViewPairHash> distances_between_stops_;
};

}  // namespace catalog