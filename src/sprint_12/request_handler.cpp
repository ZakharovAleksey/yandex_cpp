#include "request_handler.h"

#include <string>

#include "json_reader.h"

namespace request {

using namespace std::literals;
using namespace catalogue;
using namespace routing;

RequestHandler::RequestHandler(const catalogue::TransportCatalogue& db, request::ResponseSettings settings)
    : db_(db), settings_(std::move(settings)) {}

std::optional<catalogue::BusStatistics> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.GetBusStatistics(bus_name);
}

std::unique_ptr<std::set<std::string_view>> RequestHandler::GetBusesThroughTheStop(
    const std::string_view& stop_name) const {
    return db_.GetBusesPassingThroughTheStop(stop_name);
}

std::string RequestHandler::RenderMap() const {
    return render::RenderTransportMap(db_, settings_.visualization);
}

routing::ResponseDataOpt RequestHandler::BuildRoute(std::string_view from, std::string_view to) const {
    // Create router if it is still empty - crete only once
    if (!router_.has_value())
        router_.emplace(routing::TransportRouter(db_, settings_.routing));

    return router_->BuildRoute(from, to);
}

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

    RequestHandler handler_(transport_catalogue, settings);
    auto response = MakeStatisticsResponse(handler_, stat_requests);

    json::Print(json::Document{std::move(response)}, output);
}

}  // namespace request
