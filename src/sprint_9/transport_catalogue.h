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
    std::string number;
    size_t stops_count{0u};
    size_t unique_stops_count{0u};
    double rout_length{0.};
};

class TransportCatalogue {
public:  // Constructors
    TransportCatalogue() = default;

public:  // Methods
    void AddStop(Stop stop);
    void AddBus(Bus bus);

    [[nodiscard]] std::optional<BusStatistics> GetBusStatistics(std::string bus_number) const;

private:  // Fields
    std::deque<Stop> stops_storage_;
    std::unordered_map<std::string_view, const Stop*> stops_;

    std::deque<Bus> buses_storage_;
    std::unordered_map<std::string, const Bus*> buses_;
};

}  // namespace catalog