#pragma once

#include <deque>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "domain.h"
#include "graph.h"
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
};

/// @brief Response in the route corresponding to the bus
struct BusResponse {
    double time{0.};
    std::string type{"Bus"};

    std::string bus;
    int span_count{0};
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
    void MakeStopToVertexCorrespondence(const std::set<std::string_view>& stops);
    void AddCircleBusRoute(const catalogue::Bus& bus);
    void AddTwoDirectionalBusRoute(const catalogue::Bus& bus);

    void BuildRoutesGraph(const std::deque<catalogue::Bus>& buses);

private:
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
    std::unordered_map<graph::VertexId, std::string_view> vertex_to_stop_;

    std::unordered_map<graph::Edge<Weight>, ResponseItem, EdgeHash> edge_response_;

    std::unique_ptr<Graph> routes_{nullptr};
    std::unique_ptr<Router> router_{nullptr};
};

using TransportRouterOpt = std::optional<TransportRouter>;

}  // namespace routing