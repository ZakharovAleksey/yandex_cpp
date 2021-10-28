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

        stop_to_vertex_.emplace(stop, StopRepresentation{.start_ = start, .end_ = end});
        vertex_to_stop_.emplace(start, stop);
        vertex_to_stop_.emplace(end, stop);
    }
}

void TransportRouter::AddCircleBusRoute(const catalogue::Bus& bus) {
    const auto& distances = catalogue_.GetAllDistancesOnTheRoute(bus.number);
    const auto& stops = bus.stop_names;

    graph::VertexId from{0};
    graph::VertexId to{0};

    for (int id_from = 0; id_from != stops.size(); ++id_from) {
        from = stop_to_vertex_[stops[id_from]].end_;

        for (int id_to = id_from + 1; id_to != stops.size(); ++id_to) {
            to = stop_to_vertex_[stops[id_to]].start_;

            // clang-format off
            routes_->AddEdge(graph::Edge<Weight>{
                .from = from,
                .to = to,
                .weight = distances.at({stops[id_from], stops[id_to]}) / settings_.bus_velocity_
            });
            // clang-format on
        }
    }
}

void TransportRouter::AddTwoDirectionalBusRoute(const catalogue::Bus& bus) {
    const auto& distances = catalogue_.GetAllDistancesOnTheRoute(bus.number);
    const auto& stops = bus.stop_names;

    graph::VertexId from{0};
    graph::VertexId to{0};

    for (int id_from = 0; id_from != stops.size(); ++id_from) {
        from = stop_to_vertex_[stops[id_from]].end_;

        for (int id_to = 0; id_to != stops.size(); ++id_to) {
            if (id_from == id_to)
                continue;

            to = stop_to_vertex_[stops[id_to]].start_;

            // clang-format off
            routes_->AddEdge(graph::Edge<Weight>{
                .from = from,
                .to = to,
                .weight = distances.at({stops[id_from], stops[id_to]}) / settings_.bus_velocity_
            });
            // clang-for\mat on

            // TODO: update inter_stops_moves_
        }
    }
}

void TransportRouter::BuildRoutesGraph(const std::deque<catalogue::Bus>& buses) {
    routes_ = std::make_unique<Graph>(vertex_to_stop_.size() + 1);

    // Step 1. Create "wait"-type edges for each stop
    // clang-format off
    for (const auto& [_, stop_vertexes] : stop_to_vertex_)
        routes_->AddEdge(graph::Edge<double>{
            .from = stop_vertexes.start_,
            .to = stop_vertexes.end_,
            .weight = static_cast<double>(settings_.bus_wait_time_)
        });
    // clang-format on

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

            // This is "wait"
            if (edge.from % 2 == 1 && edge.to % 2 == 0) {
                // clang-format off
                response->items_.emplace_back(ResponseItem{
                    .type_ = "Wait",
                    .time_ = edge.weight,
                    .stop_name_ = std::make_optional<std::string>(vertex_to_stop_.at(edge.from))
                });
                // clang-format on
            } else {
                // clang-format off
                response->items_.emplace_back(ResponseItem{
                    .type_ = "Bus",
                    .time_ = edge.weight,
                    .bus_ = "Some BUS",
                    .span_count_ = 2
                });
                // clang-format on
            }
        }
    }

    return response;
}

}  // namespace routing