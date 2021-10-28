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
    /// Velocity in km per hour
    double bus_velocity_{0};
    /// Wait time in minutes
    int bus_wait_time_{0};
};

struct WaitResponse {
    double time{0.};
    std::string type{"Wait"};

    std::string stop_name;
};

struct BusResponse {
    double time{0.};
    std::string type{"Bus"};

    std::string bus;
    int span_count{0};
};

using ResponseItem = std::variant<WaitResponse, BusResponse>;

using RouteItems = std::vector<ResponseItem>;

struct ResponseData {
    double total_time_{0.};
    RouteItems items_;
};

using ResponseDataOpt = std::optional<ResponseData>;

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
    struct WayToMove {
        std::string_view bus_;
        int stops_count_{0};
    };

    using PossibleWaysToMove = std::vector<WayToMove>;

    struct StopRepresentation {
        graph::VertexId start_{0};
        graph::VertexId end_{0};
    };

    struct WayRepresentation {
        graph::VertexId from_{0};
        graph::VertexId to_{0};
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

    std::unordered_map<std::string_view, StopRepresentation> stop_to_vertex_;
    std::unordered_map<graph::VertexId, std::string_view> vertex_to_stop_;

    // TODO: use it during the graph creation
    std::unordered_map<graph::Edge<Weight>, ResponseItem, EdgeHash> edge_response_;

    std::unique_ptr<Graph> routes_{nullptr};
    std::unique_ptr<Router> router_{nullptr};
};

using TransportRouterOpt = std::optional<TransportRouter>;

}  // namespace routing