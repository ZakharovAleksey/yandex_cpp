#include "transport_router.h"

namespace routing {

TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue, Settings settings)
    : catalogue_(catalogue), settings_(settings) {
    BuildVertexesForStops(catalogue.GetUniqueStops());
    BuildRoutesGraph(catalogue.GetBuses());

    router_ = std::make_unique<Router>(*routes_);
}

void TransportRouter::BuildVertexesForStops(const std::set<std::string_view>& stops) {
    graph::VertexId start{0};
    graph::VertexId end{1};

    stop_to_vertex_.reserve(stops.size());

    for (std::string_view stop : stops) {
        stop_to_vertex_.emplace(stop, StopVertexes{start, end});

        // Stops have 'begin' and 'end' so the ids difference is count('begin', 'end') = 2
        start += 2;
        end += 2;
    }
}

void TransportRouter::AddBusRouteEdges(const catalogue::Bus& bus) {
    const auto& distances = catalogue_.GetAllDistancesOnTheRoute(bus.number, settings_.bus_velocity_);

    graph::VertexId from{0};
    graph::VertexId to{0};

    for (const auto& [route, info] : distances) {
        from = stop_to_vertex_[route.first].end;
        to = stop_to_vertex_[route.second].start;

        auto edge = graph::Edge<Weight>{from, to, info.time};

        routes_->AddEdge(edge);
        edge_response_.emplace(edge, BusResponse(info.time, bus.number, info.stops_count));
    }
}

void TransportRouter::BuildRoutesGraph(const std::deque<catalogue::Bus>& buses) {
    routes_ = std::make_unique<Graph>(stop_to_vertex_.size() * 2);

    // Step 1. Create "wait"-type edges for each stop
    auto wait_time = static_cast<double>(settings_.bus_wait_time_);
    auto stop_edge = graph::Edge<Weight>{};

    for (auto [stop_name, stop_vertexes] : stop_to_vertex_) {
        stop_edge = graph::Edge<Weight>{stop_vertexes.start, stop_vertexes.end, wait_time};

        routes_->AddEdge(stop_edge);
        edge_response_.emplace(stop_edge, WaitResponse(wait_time, stop_name));
    }

    // Step 2. Add "bus"-type edges for each stop in bus route
    for (const auto& bus : buses)
        AddBusRouteEdges(bus);
}

ResponseDataOpt TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    ResponseDataOpt response{std::nullopt};

    graph::VertexId id_from = stop_to_vertex_.at(from).start;
    graph::VertexId id_to = stop_to_vertex_.at(to).start;

    if (auto route = router_->BuildRoute(id_from, id_to)) {
        response.emplace(ResponseData{});
        response->total_time = route->weight;

        for (auto edge_id : route->edges) {
            graph::Edge<Weight> edge = routes_->GetEdge(edge_id);

            response->items.emplace_back(edge_response_.at(edge));
        }
    }

    return response;
}

}  // namespace routing