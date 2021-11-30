#pragma once

#include <transport_catalogue.pb.h>

#include <filesystem>

#include "transport_catalogue.h"

namespace serialization {

using Path = std::filesystem::path;
using DistanceBetweenStops = std::pair<catalogue::TransportCatalogue::StopPointersPair, int>;

class SerializationManager {
public:  // Constructors
    explicit SerializationManager(const catalogue::TransportCatalogue& catalogue);

    const proto_catalogue::TransportCatalogue& GetTransportCatalogue() const;
private:  // Methods
    proto_catalogue::Coordinates SerializeCoordinates(const geo::Coordinates& coordinates);
    proto_catalogue::Stop SerializeStop(const catalogue::Stop& stop);
    proto_catalogue::Distance SerializeDistance(const DistanceBetweenStops& distance_between_stops);
    proto_catalogue::Bus SerializeBus(const catalogue::Bus& bus);

private:  // Fields
    proto_catalogue::TransportCatalogue catalogue_;
};

bool SerializeTransportCatalogue(const Path& storage_path, const catalogue::TransportCatalogue& catalogue);

catalogue::TransportCatalogue DeserializeTransportCatalogue(const Path& load_path);

}  // namespace serialization