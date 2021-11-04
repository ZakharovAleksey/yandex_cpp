#pragma once

/*
 * Description: parses JSON data built during parsing and forms an array
 * of JSON responses
 */

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace request {

catalogue::TransportCatalogue ProcessBaseRequest(const json::Array& requests);

render::Visualization ParseVisualizationSettings(const json::Dict& settings);

routing::Settings ParseRoutingSettings(const json::Dict& requests);

json::Node MakeStatisticsResponse(request::RequestHandler& handler, const json::Array& requests);

}  // namespace request