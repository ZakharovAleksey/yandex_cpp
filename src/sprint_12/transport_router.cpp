#include "transport_router.h"

namespace routing {

TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue, Settings settings)
    : catalogue_(catalogue), settings_(settings) {
    MakeStopToVertexCorrespondence(catalogue.GetUniqueStops());
    BuildRoutesGraph(catalogue.GetBuses());

    router_ = std::make_unique<Router>(*routes_);
}

void TransportRouter::MakeStopToVertexCorrespondence(const std::set<std::string_view>& stops) {
    graph::VertexId start{0};
    graph::VertexId end{0};

    for (std::string_view stop : stops) {
        start = end + 1;
        end = start + 1;

        stop_to_vertex_.emplace(stop, StopRepresentation{start, end});
        vertex_to_stop_.emplace(start, stop);
        vertex_to_stop_.emplace(end, stop);
    }
}

void TransportRouter::AddCircleBusRoute(const catalogue::Bus& bus) {
    const auto& distances = catalogue_.GetAllDistancesOnTheRoute(bus.number, settings_.bus_velocity_);

    graph::VertexId from{0};
    graph::VertexId to{0};

    for (const auto& [route, info] : distances) {
        from = stop_to_vertex_[route.first].end_;
        to = stop_to_vertex_[route.second].start_;

        auto edge = graph::Edge<Weight>{from, to, info.time};
        routes_->AddEdge(edge);

        edge_response_.emplace(edge, BusResponse{info.time, "Bus", bus.number, info.stops_count});
    }
}

void TransportRouter::AddTwoDirectionalBusRoute(const catalogue::Bus& bus) {
    const auto& distances = catalogue_.GetAllDistancesOnTheRoute(bus.number, settings_.bus_velocity_);

    graph::VertexId from{0};
    graph::VertexId to{0};

    for (const auto& [route, info] : distances) {
        from = stop_to_vertex_[route.first].end_;
        to = stop_to_vertex_[route.second].start_;

        auto edge = graph::Edge<Weight>{from, to, info.time};
        routes_->AddEdge(edge);

        edge_response_.emplace(edge, BusResponse{info.time, "Bus", bus.number, info.stops_count});
    }
}

void TransportRouter::BuildRoutesGraph(const std::deque<catalogue::Bus>& buses) {
    routes_ = std::make_unique<Graph>(vertex_to_stop_.size() + 1);

    // Step 1. Create "wait"-type edges for each stop

    for (auto [stop_name, stop_vertexes] : stop_to_vertex_) {
        // clang-format off
        auto edge = graph::Edge<Weight>{
            stop_vertexes.start_,
            stop_vertexes.end_, // TODO: mb remove all static_casts for time
            static_cast<double>(settings_.bus_wait_time_)
        };
        // clang-format on

        routes_->AddEdge(edge);
        edge_response_.emplace(edge, WaitResponse{
                                         static_cast<double>(settings_.bus_wait_time_),
                                         "Wait",
                                         std::string(stop_name),
                                     });
    }

    // Step 2. Add "bus"-type edges for each stop in bus route
    for (const auto& bus : buses) {
        if (bus.type == catalogue::RouteType::TWO_DIRECTIONAL) {
            AddTwoDirectionalBusRoute(bus);
        } else {
            AddCircleBusRoute(bus);
        }
    }
}

ResponseDataOpt TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    ResponseDataOpt response{std::nullopt};

    graph::VertexId id_from = stop_to_vertex_.at(from).start_;
    graph::VertexId id_to = stop_to_vertex_.at(to).start_;

    if (auto path = router_->BuildRoute(id_from, id_to)) {
        response.emplace(ResponseData{});
        response->total_time_ = path->weight;

        for (auto edge_id : path->edges) {
            graph::Edge<Weight> edge = routes_->GetEdge(edge_id);

            response->items_.emplace_back(edge_response_.at(edge));
        }
    }

    return response;
}

}  // namespace routing