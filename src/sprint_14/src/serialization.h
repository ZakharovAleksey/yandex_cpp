#pragma once

#include <transport_catalogue.pb.h>

#include <filesystem>

#include "transport_catalogue.h"

namespace serialization {

using Path = std::filesystem::path;
using DistanceBetweenStops = std::pair<catalogue::TransportCatalogue::StopPointersPair, int>;
using StopToIdCorrespondence = std::unordered_map<std::string_view, int>;
using IdToStopsCorrespondence = std::unordered_map<int, std::string_view>;

struct StopsDistanceInfo {
    std::string from;
    std::string to;
    int distance;
};

class SerializationManager {
public:  // Constructors
    explicit SerializationManager(const catalogue::TransportCatalogue& catalogue);

    const proto_catalogue::TransportCatalogue& GetTransportCatalogue() const;

private:  // Methods
    proto_catalogue::Coordinates SerializeCoordinates(const geo::Coordinates& coordinates);
    proto_catalogue::Stop SerializeStop(const catalogue::Stop& stop);
    proto_catalogue::Distance SerializeDistance(const DistanceBetweenStops& distance_between_stops, const StopToIdCorrespondence& stop_to_id);
    proto_catalogue::Bus SerializeBus(const catalogue::Bus& bus, const StopToIdCorrespondence& stop_to_id);
private:  // Fields
    proto_catalogue::TransportCatalogue catalogue_;
};

class DeserializationManager {
public:  // Constructor
    explicit DeserializationManager(const proto_catalogue::TransportCatalogue& object);

    const catalogue::TransportCatalogue& GetTransportCatalogue() const;

private:  // Methods
    geo::Coordinates DeserializeCoordinates(const proto_catalogue::Coordinates& object);
    catalogue::Stop DeserializeStop(const proto_catalogue::Stop& object);
    StopsDistanceInfo DeserializeDistance(const proto_catalogue::Distance& object, const IdToStopsCorrespondence& id_to_stop);
    catalogue::Bus DeserializeBus(const proto_catalogue::Bus& object, const IdToStopsCorrespondence& id_to_stop);

private:  // Fields
    catalogue::TransportCatalogue catalogue_;
};

bool SerializeTransportCatalogue(const Path& storage_path, const catalogue::TransportCatalogue& catalogue);

catalogue::TransportCatalogue DeserializeTransportCatalogue(const Path& load_path);

}  // namespace serialization