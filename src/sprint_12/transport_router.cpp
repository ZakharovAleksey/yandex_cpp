#include "transport_router.h"

namespace routing {

TransportRouter::TransportRouter(Settings settings, const TransportRouterInputData& data)
    : settings_(settings), distances_(data.distances_) {
    MakeStopToVertexCorrespondence(data.stops_);
    BuildRoutesGraph(data.buses_);
}

double TransportRouter::CalculateWeight(std::string_view from, std::string_view to) const {
    return distances_.at({from, to}) / settings_.bus_velocity_;
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
    const auto& stops = bus.stop_names;

    graph::VertexId from{0};
    graph::VertexId to{0};
    // TODO: add circle A2 -> A1

    for (int id_from = 0; id_from != stops.size(); ++id_from) {
        from = stop_to_vertex_[stops[id_from]].end_;

        for (int id_to = (id_from + 1) % stops.size(); id_to != stops.size(); ++id_to) {
            to = stop_to_vertex_[stops[id_to]].start_;

            // clang-format off
            routes_.AddEdge(graph::Edge<double>{
                .from = from,
                .to = to,
                .weight = CalculateWeight(stops[id_from], stops[id_to])
            });
            // clang-format on

            // TODO: update inter_stops_moves_

            // HERE ALL
            if (id_from == stops.size() - 1 && id_to == 0)
                break;
        }
    }
}

void TransportRouter::AddTwoDirectionalBusRoute(const catalogue::Bus& bus) {
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
            routes_.AddEdge(graph::Edge<double>{
                .from = from,
                .to = to,
                .weight = CalculateWeight(stops[id_from], stops[id_to])
            });
            // clang-format on

            // TODO: update inter_stops_moves_
        }
    }
}

void TransportRouter::BuildRoutesGraph(const std::deque<catalogue::Bus>& buses) {
    // Step 1. Create "wait"-type edges for each stop
    // clang-format off
    for (const auto& [_, stop_vertexes] : stop_to_vertex_)
        routes_.AddEdge(graph::Edge<double>{
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

RouteItems TransportRouter::FindRoute(std::string_view from, std::string_view to) const {
    RouteItems route;

    return route;
}

double TransportRouter::MeasureTimeOnTheRoute(const RouteItems& route) const {
    double total_time{0.};

    return total_time;
}

ResponseDataOpt TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    ResponseData response;

    response.items_ = FindRoute(from, to);
    response.total_time_ = MeasureTimeOnTheRoute(response.items_);

    return !response.items_.empty() ? std::make_optional<ResponseData>(response) : std::nullopt;
}

}  // namespace routing