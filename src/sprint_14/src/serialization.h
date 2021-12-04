#pragma once

#include "graph.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace serialization {

void SerializeTransportCatalogue(std::ofstream& output, const catalogue::TransportCatalogue& catalogue);
catalogue::TransportCatalogue DeserializeTransportCatalogue(const catalogue::Path& path);

void SerializeVisualizationSettings(std::ofstream& output, const render::Visualization& settings);
render::Visualization DeserializeVisualizationSettings(const catalogue::Path& path);

void SerializeTransportRouter(std::ofstream& output, const routing::TransportRouter& router);
routing::TransportRouter DeserializeTransportRouter(const catalogue::Path& path, const catalogue::TransportCatalogue& catalogue);

};  // namespace serialization
