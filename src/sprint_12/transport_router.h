#pragma once

#include <deque>
#include <optional>
#include <string_view>
#include <vector>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace routing {

struct Settings {
    /// Velocity in km per hour
    int bus_velocity_{0};
    /// Wait time in minutes
    int bus_wait_time_{0};
};

struct ResponseItem {
    std::string type_;
    double time_;

    /* For the "wait" response */
    std::optional<std::string> stop_name_{std::nullopt};

    /* For the "bus" response */
    std::optional<std::string> bus_{std::nullopt};
    std::optional<int> span_count_{std::nullopt};
};

using RouteItems = std::vector<ResponseItem>;

struct ResponseData {
    double total_time_{0.};
    RouteItems items_;
};

using ResponseDataOpt = std::optional<ResponseData>;

/// @brief Stories all information, necessary to build the Transport Router
struct TransportRouterInputData {
    std::set<std::string_view> stops_;
    std::deque<catalogue::Bus> buses_;
    catalogue::InterStopsStorage<int> distances_;
};

class TransportRouter {
public:  // Constructor
    TransportRouter(Settings settings, const TransportRouterInputData& data) : settings_(settings) {}

public:  // Methods
    [[nodiscard]] ResponseDataOpt BuildRoute(std::string_view from, std::string_view to) const;

private:  // Methods
    [[nodiscard]] RouteItems FindRoute(std::string_view from, std::string_view to) const;
    [[nodiscard]] double MeasureTimeOnTheRoute(const RouteItems& route) const;

    void AddCircleBusRoute(const catalogue::Bus& bus){};
    void AddTwoDirectionalBusRoute(const catalogue::Bus& bus){};

private:
    struct WayToMove {
        std::string_view bus_;
        int stops_count_{0};
    };

    using PossibleWaysToMove = std::vector<WayToMove>;

private:  // Fields
    Settings settings_;

    std::unordered_map<std::string_view, graph::VertexId> stop_to_vertex_;
    std::unordered_map<graph::VertexId, std::string_view> vertex_to_stop_;
    catalogue::InterStopsStorage<PossibleWaysToMove> inter_stops_moves_;

    graph::DirectedWeightedGraph<double> routes_;
};

using TransportRouterOpt = std::optional<TransportRouter>;

}  // namespace routing