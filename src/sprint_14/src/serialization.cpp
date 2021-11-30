#include "serialization.h"

#include <fstream>

namespace serialization {

/* SERIALIZATION */

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

    std::ofstream out(storage_path.c_str(), std::ofstream::binary);
    object.SerializeToOstream(&out);

    return true;
}

/* DESERIALIZATION */

DeserializationManager::DeserializationManager(const proto_catalogue::TransportCatalogue& object) {
    // !!! IMPORTANT !!! Do not change the order: STOPS should be before DISTANCES and BUSES
    // The same order as in ProcessBaseRequest() function

    const auto& stops_object = object.stops();
    for (const auto& stop_object : stops_object)
        catalogue_.AddStop(DeserializeStop(stop_object));

    const auto& distances_object = object.distances();
    for (const auto& distance_object : distances_object) {
        auto info = DeserializeDistance(distance_object);
        catalogue_.AddDistance(info.from, info.to, info.distance);
    }

    const auto& buses_object = object.buses();
    for (const auto& bus_object : buses_object) {
        catalogue_.AddBus(DeserializeBus(bus_object));
    }
}

const catalogue::TransportCatalogue& DeserializationManager::GetTransportCatalogue() const {
    return catalogue_;
}

geo::Coordinates DeserializationManager::DeserializeCoordinates(const proto_catalogue::Coordinates& object) {
    geo::Coordinates coordinates;

    coordinates.lat = object.lat();
    coordinates.lng = object.lng();

    return coordinates;
}

catalogue::Stop DeserializationManager::DeserializeStop(const proto_catalogue::Stop& object) {
    catalogue::Stop stop;

    stop.name = object.name();
    stop.point = DeserializeCoordinates(object.point());

    return stop;
}

StopsDistanceInfo DeserializationManager::DeserializeDistance(const proto_catalogue::Distance& object) {
    StopsDistanceInfo info;

    info.from = object.from();
    info.to = object.to();
    info.distance = static_cast<int>(object.distance());

    return info;
}

catalogue::Bus DeserializationManager::DeserializeBus(const proto_catalogue::Bus& object) {
    using Route = catalogue::RouteType;

    catalogue::Bus bus;

    bus.number = object.number();
    bus.type = object.is_circle() ? Route::CIRCLE : Route::TWO_DIRECTIONAL;

    bus.stop_names.resize(object.stop_ids_size());
    for (int id = 0; id < object.stop_ids_size(); ++id)
        bus.stop_names[id] = object.stop_ids(id);

    return bus;
}

catalogue::TransportCatalogue DeserializeTransportCatalogue(const Path& load_path) {
    proto_catalogue::TransportCatalogue object;
    std::ifstream in(load_path.c_str(), std::ifstream::binary);

    if (!object.ParseFromIstream(&in)) {
        return {};
    }

    DeserializationManager manager(object);
    auto transport_catalogue = manager.GetTransportCatalogue();

    return transport_catalogue;
}

}  // namespace serialization
