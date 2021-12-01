#include "serialization.h"

#include <transport_catalogue.pb.h>

#include <fstream>

namespace serialization {

using StopNameToIdContaner = std::unordered_map<std::string_view, int>;
// TODO: maybe string_view here
using IdToStopNameContainer = std::unordered_map<int, std::string>;

namespace {
StopNameToIdContaner SetIdToEachStop(const std::deque<catalogue::Stop>& stops) {
    // !!! IMPORTANT !!! Store IDs in the direct order for SERIALIZATION
    StopNameToIdContaner result;
    result.reserve(stops.size());

    for (int id = 0; id < stops.size(); ++id)
        result.emplace(stops[id].name, id);

    return result;
}

IdToStopNameContainer SetNameToEachStop(const std::deque<catalogue::Stop>& stops) {
    // !!! IMPORTANT !!! Store IDs in the reverse order for DESERIALIZATION
    IdToStopNameContainer result;
    result.reserve(stops.size());

    for (int id = stops.size() - 1; id >= 0; --id)
        result.emplace(id, stops[id].name);

    return result;
}

}  // namespace

void SerializeTransportCatalogue(const catalogue::Path& path, const catalogue::TransportCatalogue& catalogue) {
    proto_tc::TransportCatalogue object;

    const auto& stops = catalogue.GetStops();
    const auto stop_to_id = SetIdToEachStop(stops);

    // Step 1. Serialize stops
    for (const auto& stop : stops) {
        proto_tc::Stop stop_object;

        stop_object.set_name(stop.name);
        stop_object.mutable_point()->set_lng(stop.point.lng);
        stop_object.mutable_point()->set_lat(stop.point.lat);

        object.mutable_stops()->Add(std::move(stop_object));
    }

    // Step 2. Serialize distances between stops
    const auto& distances = catalogue.GetDistancesBetweenStops();
    for (const auto& [stops_pair, distance] : distances) {
        proto_tc::DistanceBetweenStops distance_object;

        distance_object.set_from(stop_to_id.at(stops_pair.first->name));
        distance_object.set_to(stop_to_id.at(stops_pair.second->name));
        distance_object.set_distance(distance);

        object.mutable_distances()->Add(std::move(distance_object));
    }

    // Step 3. Serialize buses
    const auto& buses = catalogue.GetBuses();
    for (const auto& bus : buses) {
        proto_tc::Bus bus_object;

        bus_object.set_name(bus.number);
        bus_object.set_is_circle(bus.type == catalogue::RouteType::CIRCLE);
        for (std::string_view stop : bus.stop_names)
            bus_object.add_stops_ids(stop_to_id.at(stop));
    }

    std::ofstream output(path, std::ios::binary);
    object.SerializeToOstream(&output);
}

catalogue::TransportCatalogue DeserializeTransportCatalogue(const catalogue::Path& path) {
    return {};
}

}  // namespace serialization
