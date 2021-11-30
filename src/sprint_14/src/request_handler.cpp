#include "request_handler.h"

#include <string>

#include "json_reader.h"
#include "serialization.h"

namespace request {

using namespace std::literals;
using namespace catalogue;
using namespace routing;

namespace {
using namespace std::string_literals;

struct RequestSettings {
    inline static const std::string kRouting{"routing_settings"s};
    inline static const std::string kRender{"render_settings"s};
    inline static const std::string kSerialization{"serialization_settings"s};
};

struct RequestHeader {
    inline static const std::string kBase{"base_requests"s};
    inline static const std::string kStat{"stat_requests"s};
};

RequestType DetectRequestType(const json::Node& request_body) {
    const auto& dictionary = request_body.AsDict();
    return dictionary.count(RequestHeader::kBase) ? RequestType::MakeBase : RequestType::ProcessRequests;
}

void MakeBase(const json::Node& request_body) {
    ResponseSettings settings;

    // Step 1. Form catalogue, basing on the input
    const auto& base_requests = request_body.AsDict().at(RequestHeader::kBase).AsArray();
    auto transport_catalogue = ProcessBaseRequest(base_requests);

    // Step 2. Parse rendering settings
    const auto& render_object = request_body.AsDict().at(RequestSettings::kRender).AsDict();
    settings.visualization = ParseVisualizationSettings(render_object);

    // Step 3. Parse routing settings
    const auto& routing_object = request_body.AsDict().at(RequestSettings::kRouting).AsDict();
    settings.routing = ParseRoutingSettings(routing_object);

    // Step 4. Extract path to the db serialization
    const auto& serialization_object = request_body.AsDict().at(RequestSettings::kSerialization).AsDict();
    settings.path_to_database = Path(ParseSerializationSettings(serialization_object));

    serialization::SerializeTransportCatalogue(settings.path_to_database, transport_catalogue);
}

void ProcessRequests(const json::Node& request_body, std::ostream& output) {
    ResponseSettings settings;

    // Step 1. Extract path to the serialized database
    const auto& serialization_object = request_body.AsDict().at(RequestSettings::kSerialization).AsDict();
    settings.path_to_database = Path(ParseSerializationSettings(serialization_object));

    const auto transport_catalogue = serialization::DeserializeTransportCatalogue(settings.path_to_database);

    const auto& stat_requests = request_body.AsDict().at("stat_requests"s).AsArray();

    RequestHandler handler_(transport_catalogue, settings);
    auto response = MakeStatisticsResponse(handler_, stat_requests);

    json::Print(json::Document{std::move(response)}, output);
}

}  // namespace

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
    const auto input_json = json::Load(input).GetRoot();
    auto type = DetectRequestType(input_json);

    switch (type) {
        case RequestType::MakeBase:
            MakeBase(input_json);
            break;
        case RequestType::ProcessRequests:
            ProcessRequests(input_json, output);
            break;
    }
}

}  // namespace request
