#include "serialization.h"

#include <fstream>

namespace serialization {

SerializationManager::SerializationManager(const catalogue::TransportCatalogue& catalogue) {
    for (const auto& stop : catalogue.GetStops())
        catalogue_.mutable_stops()->Add(SerializeStop(stop));

    for (const auto& bus : catalogue.GetBuses())
        catalogue_.mutable_buses()->Add(SerializeBus(bus));

    for (const auto& distance_info : catalogue.GetDistancesBetweenStops())
        catalogue_.mutable_distances()->Add(SerializeDistance(distance_info));
}

const proto_catalogue::TransportCatalogue& SerializationManager::GetTransportCatalogue() const {
    return catalogue_;
}

proto_catalogue::Coordinates SerializationManager::SerializeCoordinates(const geo::Coordinates& coordinates) {
    proto_catalogue::Coordinates object;

    object.set_lat(coordinates.lat);
    object.set_lng(coordinates.lng);

    return object;
}

proto_catalogue::Stop SerializationManager::SerializeStop(const catalogue::Stop& stop) {
    proto_catalogue::Stop object;

    object.set_name(stop.name);
    *object.mutable_point() = SerializeCoordinates(stop.point);

    return object;
}

proto_catalogue::Distance SerializationManager::SerializeDistance(const DistanceBetweenStops& distance_between_stops) {
    proto_catalogue::Distance object;

    auto& [stops, distance] = distance_between_stops;
    object.set_from(stops.first->name);
    object.set_to(stops.second->name);

    object.set_distance(distance);

    return object;
}

proto_catalogue::Bus SerializationManager::SerializeBus(const catalogue::Bus& bus) {
    proto_catalogue::Bus object;

    object.set_number(bus.number);
    object.set_is_circle(bus.type == catalogue::RouteType::CIRCLE);

    for (const auto& stop : bus.stop_names)
        object.add_stop_ids(stop.data());

    return object;
}

bool SerializeTransportCatalogue(const Path& storage_path, const catalogue::TransportCatalogue& catalogue) {
    SerializationManager manager(catalogue);
    const auto& object = manager.GetTransportCatalogue();

    std::ofstream out(storage_path.c_str(), std::ostream::binary);
    object.SerializeToOstream(&out);

    return true;
}

catalogue::TransportCatalogue DeserializeTransportCatalogue(const Path& load_path) {
    return {};
}

}  // namespace serialization
