#include "transport_router.h"

namespace routing {

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