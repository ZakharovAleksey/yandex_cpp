#pragma once

/*
 * Description: parses JSON data built during parsing and forms an array
 * of JSON responses
 */

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace request {

catalogue::TransportCatalogue ProcessBaseRequest(const json::Array& requests);

render::Visualization ParseVisualizationSettings(const json::Dict& settings);

routing::Settings ParseRoutingSettings(const json::Dict& requests);

json::Node MakeStatResponse(const catalogue::TransportCatalogue& catalogue, const json::Array& requests,
                            const render::Visualization& visualization_settings, const routing::Settings& routing_settings);


}  // namespace request