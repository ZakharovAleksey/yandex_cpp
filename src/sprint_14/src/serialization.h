#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

namespace serialization {

void SerializeTransportCatalogue(std::ofstream& output, const catalogue::TransportCatalogue& catalogue);
catalogue::TransportCatalogue DeserializeTransportCatalogue(const catalogue::Path& path);

void SerializeVisualizationSettings(std::ofstream& output, const render::Visualization& settings);
render::Visualization DeserializeVisualizationSettings(const catalogue::Path& path);

};  // namespace serialization
