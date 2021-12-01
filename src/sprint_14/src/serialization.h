#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

namespace serialization {

void SerializeTransportCatalogue(const catalogue::Path& path, const catalogue::TransportCatalogue& catalogue);
catalogue::TransportCatalogue DeserializeTransportCatalogue(const catalogue::Path& path);

void SerializeVisualizationSettings(const catalogue::Path& path, const render::Visualization& settings);
render::Visualization DeserializeVisualizationSettings(const catalogue::Path& path);

};  // namespace serialization
