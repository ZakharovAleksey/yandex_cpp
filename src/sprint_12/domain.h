#pragma once

/*
 * Description: classes of the main entities of the subject area,
 * describe buses and stops
 */

#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"

namespace catalogue {

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

private:  // Fields
    static const size_t even_value{37};
};

using StopPointersPair = std::pair<std::shared_ptr<Stop>, std::shared_ptr<Stop>>;

struct BusStatistics {
    std::string_view number;
    size_t stops_count{0u};
    size_t unique_stops_count{0u};
    int rout_length{0};
    double curvature{0.};
};

std::ostream& operator<<(std::ostream& os, const BusStatistics& statistics);
}  // namespace catalogue