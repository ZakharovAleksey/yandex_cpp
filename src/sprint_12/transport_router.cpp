#include "transport_router.h"

namespace routing {

void TransportRouter::SetSettings(Settings settings) {
    settings_ = settings;
}

RouteItems TransportRouter::FindRoute(std::string_view from, std::string_view to) const {
    RouteItems route;

    return route;
}

double TransportRouter::MeasureTimeOnTheRoute(const RouteItems& route) const {
    double total_time{0.};

    return total_time;
}

RouteResponse TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    RouteItems route = FindRoute(from, to);

    // clang-format off
    return RouteResponse {
        .total_time_ = MeasureTimeOnTheRoute(route),
        .items_ = std::move(route)
    };
    // clang-format on
}
}  // namespace routing