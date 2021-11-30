#include "serialization.h"

#include <fstream>

namespace serialization {

namespace {
StopToIdCorrespondence StopsToIds(const std::deque<catalogue::Stop>& stops) {
    StopToIdCorrespondence result;
    result.reserve(stops.size());

    int id{0};
    for (const auto& stop : stops)
        result.emplace(stop.name, id++);

    return result;
}

IdToStopsCorrespondence IdsToStops(const std::deque<catalogue::Stop>& stops) {
    IdToStopsCorrespondence result;
    result.reserve(stops.size());

    int id = stops.size() - 1;
    for (const auto& stop : stops)
        result.emplace(id--, stop.name);

    return result;
}

}  // namespace

/* SERIALIZATION */

SerializationManager::SerializationManager(const catalogue::TransportCatalogue& catalogue) {
    const auto& stops = catalogue.GetStops();
    auto stop_to_id = StopsToIds(stops);

//    std::cout << "STOP TO ID " << std::endl;
//    for (const auto& [key, value] : stop_to_id) {
//        std::cout << key << " -> " << value << std::endl;
//    }

    for (const auto& stop : stops)
        catalogue_.mutable_stops()->Add(SerializeStop(stop));

    for (const auto& bus : catalogue.GetBuses())
        catalogue_.mutable_buses()->Add(SerializeBus(bus, stop_to_id));

    for (const auto& distance_info : catalogue.GetDistancesBetweenStops())
        catalogue_.mutable_distances()->Add(SerializeDistance(distance_info, stop_to_id));
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

proto_catalogue::Distance SerializationManager::SerializeDistance(const DistanceBetweenStops& distance_between_stops,
                                                                  const StopToIdCorrespondence& stop_to_id) {
    proto_catalogue::Distance object;

    auto& [stops, distance] = distance_between_stops;
    object.set_from(stop_to_id.at(stops.first->name));
    object.set_to(stop_to_id.at(stops.second->name));

    object.set_distance(distance);

    return object;
}

proto_catalogue::Bus SerializationManager::SerializeBus(const catalogue::Bus& bus,
                                                        const StopToIdCorrespondence& stop_to_id) {
    proto_catalogue::Bus object;

    object.set_number(bus.number);
    object.set_is_circle(bus.type == catalogue::RouteType::CIRCLE);

    for (const auto& stop : bus.stop_names)
        object.add_stop_ids(stop_to_id.at(stop));

    return object;
}

bool SerializeTransportCatalogue(const Path& storage_path, const catalogue::TransportCatalogue& catalogue) {
    SerializationManager manager(catalogue);
    const auto& object = manager.GetTransportCatalogue();

    std::ofstream out(storage_path, std::ios::binary);
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

    const auto ids_to_stops = IdsToStops(catalogue_.GetStops());

//    std::cout << "\n\nID TO STOP " << std::endl;
//    for (const auto& [key, value] : ids_to_stops) {
//        std::cout << key << " -> " << value << std::endl;
//    }

    const auto& distances_object = object.distances();
    for (const auto& distance_object : distances_object) {
        auto info = DeserializeDistance(distance_object, ids_to_stops);
        catalogue_.AddDistance(info.from, info.to, info.distance);
    }

    const auto& buses_object = object.buses();
    for (const auto& bus_object : buses_object) {
        catalogue_.AddBus(DeserializeBus(bus_object, ids_to_stops));
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

StopsDistanceInfo DeserializationManager::DeserializeDistance(const proto_catalogue::Distance& object,
                                                              const IdToStopsCorrespondence& id_to_stop) {
    StopsDistanceInfo info;

    info.from = id_to_stop.at(static_cast<int>(object.from()));
    info.to = id_to_stop.at(static_cast<int>(object.to()));
    info.distance = static_cast<int>(object.distance());

    return info;
}

catalogue::Bus DeserializationManager::DeserializeBus(const proto_catalogue::Bus& object,
                                                      const IdToStopsCorrespondence& id_to_stop) {
    using Route = catalogue::RouteType;

    catalogue::Bus bus;

    bus.number = object.number();
    bus.type = object.is_circle() ? Route::CIRCLE : Route::TWO_DIRECTIONAL;

    bus.stop_names.resize(object.stop_ids_size());
    for (int id = 0; id < object.stop_ids_size(); ++id)
        bus.stop_names[id] = id_to_stop.at(static_cast<int>(object.stop_ids(id)));

    return bus;
}

catalogue::TransportCatalogue DeserializeTransportCatalogue(const Path& load_path) {
    proto_catalogue::TransportCatalogue object;
    std::ifstream in(load_path, std::ios::binary);

    object.ParseFromIstream(&in);

    return DeserializationManager(object).GetTransportCatalogue();
}

}  // namespace serialization
