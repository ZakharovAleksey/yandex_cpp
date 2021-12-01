#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace serialization {

void SerializeTransportCatalogue(std::ofstream& output, const catalogue::TransportCatalogue& catalogue);
catalogue::TransportCatalogue DeserializeTransportCatalogue(const catalogue::Path& path);

void SerializeVisualizationSettings(std::ofstream& output, const render::Visualization& settings);
render::Visualization DeserializeVisualizationSettings(const catalogue::Path& path);

void SerializeRoutingSettings(std::ofstream& output, const routing::Settings& settings);
routing::Settings DeserializeRoutingSettings(const catalogue::Path& path);

};  // namespace serialization
