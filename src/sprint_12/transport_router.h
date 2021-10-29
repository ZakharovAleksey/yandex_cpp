#pragma once

#include <variant>

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

namespace routing {

struct Settings {
    double bus_velocity_{0};  // velocity in meters to minutes
    int bus_wait_time_{0};    // time in minutes
};

/* TRANSPORT ROUTER RESPONSE FORMAT */

/// @brief Response in the route corresponding to the stop
struct WaitResponse {
    double time{0.};
    std::string type{"Wait"};

    std::string stop_name;

    WaitResponse(double time, std::string_view stop) : time(time), stop_name(stop) {}
};

/// @brief Response in the route corresponding to the bus
struct BusResponse {
    double time{0.};
    std::string type{"Bus"};

    std::string bus;
    int span_count{0};

    BusResponse(double time, const std::string& bus, int stops_count) : time(time), bus(bus), span_count(stops_count) {}
};

using ResponseItem = std::variant<WaitResponse, BusResponse>;

struct ResponseData {
    double total_time{0.};
    std::vector<ResponseItem> items;
};

using ResponseDataOpt = std::optional<ResponseData>;

/* TRANSPORT ROUTER CLASS */

class TransportRouter {
public:
    using Weight = double;
    using Graph = graph::DirectedWeightedGraph<Weight>;
    using Router = graph::Router<Weight>;

public:  // Constructor
    TransportRouter(const catalogue::TransportCatalogue& catalogue, Settings settings);

public:  // Methods
    [[nodiscard]] ResponseDataOpt BuildRoute(std::string_view from, std::string_view to) const;

private:  // Methods
    void BuildVertexesForStops(const std::set<std::string_view>& stops);
    void AddBusRouteEdges(const catalogue::Bus& bus);

    void BuildRoutesGraph(const std::deque<catalogue::Bus>& buses);

private:  // Types
    struct StopVertexes {
        graph::VertexId start{0};
        graph::VertexId end{0};
    };

    struct EdgeHash {
        inline size_t operator()(const graph::Edge<Weight>& edge) const {
            // clang-format off
            return even * std::hash<size_t>{}(edge.from) +
                   even * even * std::hash<size_t>{}(edge.to) +
                   even * even * even * std::hash<Weight>{}(edge.weight);
            // clang-format on
        }

    private:
        static constexpr size_t even{41};
    };

private:  // Fields
    const catalogue::TransportCatalogue& catalogue_;
    Settings settings_;

    std::unordered_map<std::string_view, StopVertexes> stop_to_vertex_;
    std::unordered_map<graph::Edge<Weight>, ResponseItem, EdgeHash> edge_to_response_;

    /// @brief Graph, which stores all possible routes for the given TransportCatalogue
    /// @details Each stop in graph consists from the two vertexes: {start, end} to take into account passenger wait for
    /// the bus on the stop.
    /// @example Passenger arrives to stop A and moves to the stop B: A_start -> wait for the bus -> A_end -> B_start
    std::unique_ptr<Graph> routes_{nullptr};
    std::unique_ptr<Router> router_{nullptr};
};

using TransportRouterOpt = std::optional<TransportRouter>;

}  // namespace routing