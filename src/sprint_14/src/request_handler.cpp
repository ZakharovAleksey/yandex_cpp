#include "request_handler.h"

#include <fstream>
#include <string>

#include "json_reader.h"
#include "serialization.h"

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

void ProcessMakeBaseQuery(std::istream& input) {
    ResponseSettings settings;

    const auto request_body = json::Load(input).GetRoot();

    // Step 1. Get serialization path
    const auto& serialization_object = request_body.AsDict().at("serialization_settings").AsDict();
    settings.path_to_db = catalogue::Path(ParseSerializationSettings(serialization_object));

    // Step 2. Form catalogue, basing on the input
    const auto& base_requests = request_body.AsDict().at("base_requests"s).AsArray();
    auto transport_catalogue = ProcessBaseRequest(base_requests);

    // Step 3. Parse rendering settings
    const auto& render_object = request_body.AsDict().at("render_settings"s).AsDict();
    settings.visualization = ParseVisualizationSettings(render_object);

    // Step 4. Parse routing settings
    const auto& routing_object = request_body.AsDict().at("routing_settings"s).AsDict();
    settings.routing = ParseRoutingSettings(routing_object);

    // Step 5. Serialization
    std::ofstream output(settings.path_to_db, std::ios::binary);
    serialization::SerializeTransportCatalogue(output, transport_catalogue);
    serialization::SerializeVisualizationSettings(output, settings.visualization);
    // serialization::SerializeRoutingSettings(output, settings.routing);
    serialization::SerializeTransportRouter(output, TransportRouter(transport_catalogue, settings.routing));
}

void ProcessRequestsQuery(std::istream& input, std::ostream& output) {
    ResponseSettings settings;

    const auto request_body = json::Load(input).GetRoot();

    // Step 1. Get deserialization path
    const auto& serialization_object = request_body.AsDict().at("serialization_settings").AsDict();
    settings.path_to_db = catalogue::Path(ParseSerializationSettings(serialization_object));

    // Step 2. Deserialization
    const auto transport_catalogue = serialization::DeserializeTransportCatalogue(settings.path_to_db);
    settings.visualization = serialization::DeserializeVisualizationSettings(settings.path_to_db);
    settings.routing = serialization::DeserializeRoutingSettings(settings.path_to_db);

    // Step 3. Form a response
    const auto& stat_requests = request_body.AsDict().at("stat_requests"s).AsArray();

    // TODO: add router here
    RequestHandler handler_(transport_catalogue, settings);
    auto response = MakeStatisticsResponse(handler_, stat_requests);

    json::Print(json::Document{std::move(response)}, output);
}

}  // namespace request
