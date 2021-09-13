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
    geo::Coordinates point;

    [[nodiscard]] size_t Hash() const {
        return std::hash<std::string>{}(name) + even_value * std::hash<double>{}(point.lng) +
               even_value * even_value * std::hash<double>{}(point.lat);
    }

private:
    static const size_t even_value{37};
};

using StopPointersPair = std::pair<const Stop*, const Stop*>;

struct BusStatistics {
    std::string_view number;
    size_t stops_count{0u};
    size_t unique_stops_count{0u};
    int rout_length{0};
    double curvature{0.};
};

std::ostream& operator<<(std::ostream& os, const BusStatistics& statistics);

class TransportCatalogue {
public:  // Constructors
    TransportCatalogue() = default;

public:  // Methods
    void AddStop(Stop stop);
    void AddBus(Bus bus);
    void AddDistance(std::string_view stop_from, std::string_view stop_to, int distance);

    [[nodiscard]] std::optional<BusStatistics> GetBusStatistics(std::string_view bus_number) const;
    [[nodiscard]] const std::set<std::string_view>* GetBusesPassingThroughTheStop(std::string_view stop_name) const;

private:  // Types
    struct StopPointersPairHash {
        size_t operator()(const StopPointersPair& pair) const {
            return pair.first->Hash() + prime_number * pair.second->Hash();
        }

    private:
        static const size_t prime_number{31};
    };

private:  // Methods
    int CalculateRouteLength(const Bus* bus_info) const;
    double CalculateGeographicLength(const Bus* bus_info) const;

private:  // Fields
    std::deque<Stop> stops_storage_;
    std::unordered_map<std::string_view, const Stop*> stops_;

    std::deque<Bus> buses_storage_;
    std::unordered_map<std::string_view, const Bus*> buses_;

    std::unordered_map<std::string_view, std::set<std::string_view>> buses_through_stop_;
    std::unordered_map<StopPointersPair, int, StopPointersPairHash> distances_between_stops_;
};

}  // namespace catalog