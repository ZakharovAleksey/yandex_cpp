#include "request_handler.h"

#include <string>

namespace request {

using namespace std::literals;
using namespace catalogue;
using namespace routing;

void ProcessTransportCatalogueQuery(std::istream& input, std::ostream& output) {
    TransportCatalogue catalogue;
    TransportRouterOpt router{std::nullopt};
    ResponseSettings settings;

    const auto input_json = json::Load(input).GetRoot();

    // Step 1. Form catalogue, basing on the input
    const auto& base_requests = input_json.AsDict().at("base_requests"s).AsArray();
    auto transport_catalogue = ProcessBaseRequest(base_requests);

    // Step 2. Parse rendering settings
    const auto& render_object = input_json.AsDict().at("render_settings"s).AsDict();
    settings.visualization = ParseVisualizationSettings(render_object);

    // Step 3. Parse routing settings
    const auto& routing_object = input_json.AsDict().at("routing_settings"s).AsDict();
    settings.routing = ParseRoutingSettings(routing_object);

    // Step 4. Form response
    const auto& stat_requests = input_json.AsDict().at("stat_requests"s).AsArray();
    auto response = MakeStatResponse(transport_catalogue, router, stat_requests, settings);

    json::Print(json::Document{std::move(response)}, output);
}

}  // namespace request