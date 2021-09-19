#include "request_handler.h"

#include <string>

namespace request::utils {

using namespace std::literals;
using namespace catalogue;

std::pair<catalogue::Stop, bool> ParseBusStopInput(const json::Dict& info) {
    Stop stop;

    stop.name = info.at("name"s).AsString();
    stop.point.lat = info.at("latitude").AsDouble();
    stop.point.lng = info.at("longitude").AsDouble();

    bool has_road_distances = !info.at("road_distances").AsMap().empty();

    return {std::move(stop), has_road_distances};
}

Bus ParseBusRouteInput(const json::Dict& info) {
    Bus bus;

    bus.number = info.at("name"s).AsString();
    bus.type = info.at("is_roundtrip"s).AsBool() ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;

    const auto& stops = info.at("stops"s).AsArray();
    bus.stop_names.reserve(stops.size());

    for (const auto& stop : stops)
        bus.stop_names.emplace_back(stop.AsString());

    bus.unique_stops = {bus.stop_names.begin(), bus.stop_names.end()};

    return bus;
}
}  // namespace request::utils
