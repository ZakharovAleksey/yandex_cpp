#pragma once

/*
 * Description: parses JSON data built during parsing and forms an array
 * of JSON responses
 */

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace request {

struct ResponseSettings {
    routing::Settings routing;
    render::Visualization visualization;
};

catalogue::TransportCatalogue ProcessBaseRequest(const json::Array& requests);

render::Visualization ParseVisualizationSettings(const json::Dict& settings);

routing::Settings ParseRoutingSettings(const json::Dict& requests);

json::Node MakeStatResponse(const catalogue::TransportCatalogue& catalogue, routing::TransportRouterOpt& router,
                            const json::Array& requests, const ResponseSettings& settings);

}  // namespace request