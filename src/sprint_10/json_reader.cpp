#include "json_reader.h"

#include <string>

#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace request {

using namespace catalogue;
using namespace request::utils;
using namespace std::literals;

using StringPair = std::pair<std::string, std::string>;

namespace {

// TODO: mb move logic to another place
TransportCatalogue ProcessBaseRequest(const json::Array& requests) {
    TransportCatalogue catalogue;

    // We could add distances between stops ONLY when they EXIST in catalogue
    std::vector<int> requests_ids_with_road_distances;
    requests_ids_with_road_distances.reserve(requests.size());

    std::vector<int> requests_ids_with_buses;
    requests_ids_with_buses.reserve(requests.size());

    // Step 1. Store all stops to the catalog and mark requests, needed to be processed afterward
    for (int id = 0; id != requests.size(); ++id) {
        const auto& request_dict_view = requests.at(id).AsMap();

        if (request_dict_view.at("type"s) == "Stop"s) {
            auto [stop, has_road_distances] = ParseBusStopInput(request_dict_view);
            if (has_road_distances)
                requests_ids_with_road_distances.emplace_back(id);

            catalogue.AddStop(std::move(stop));
        } else if (request_dict_view.at("type"s) == "Bus"s) {
            requests_ids_with_buses.emplace_back(id);
        }
    }

    // Step 2. Add distances between all stops
    for (int id : requests_ids_with_road_distances) {
        const auto& request_dict_view = requests.at(id).AsMap();

        std::string_view stop_from = request_dict_view.at("name"s).AsString();
        for (const auto& [stop_to, distance] : request_dict_view.at("road_distances"s).AsMap())
            catalogue.AddDistance(stop_from, stop_to, distance.AsInt());
    }

    // Step 3. Add info about buses routes through stops
    // TODO: check if move works here and if YES - override everywhere
    for (int id : requests_ids_with_buses) {
        const auto& request_dict_view = requests.at(id).AsMap();
        catalogue.AddBus(ParseBusRouteInput(request_dict_view));
    }

    return catalogue;
}

json::Node MakeBusResponse(int request_id, const BusStatistics& statistics) {
    json::Dict response;

    // P.S. no need to use std::move() because all types on the right are trivial
    response.emplace("curvature"s, statistics.curvature);
    response.emplace("request_id"s, request_id);
    response.emplace("route_length"s, statistics.rout_length);
    response.emplace("stop_count"s, static_cast<int>(statistics.stops_count));
    response.emplace("unique_stop_count"s, static_cast<int>(statistics.unique_stops_count));

    return response;
}

json::Node MakeStopResponse(int request_id, const std::set<std::string_view>& buses) {
    json::Dict response;

    response.emplace("request_id"s, request_id);

    json::Array buses_array;
    buses_array.reserve(buses.size());
    for (std::string_view bus : buses)
        buses_array.emplace_back(std::string(bus));

    response.emplace("buses"s, std::move(buses_array));

    return response;
}

json::Node MakeErrorResponse(int request_id) {
    json::Dict response;

    response.emplace("request_id"s, request_id);
    response.emplace("error_message"s, "not found"s);

    return response;
}

json::Node MakeStatResponse(const TransportCatalogue& catalogue, const json::Array& requests) {
    json::Array response;
    response.reserve(requests.size());

    for (const auto& request : requests) {
        const auto& request_dict_view = request.AsMap();

        int request_id = request_dict_view.at("id"s).AsInt();
        std::string type = request_dict_view.at("type"s).AsString();
        std::string name = request_dict_view.at("name"s).AsString();

        if (type == "Bus"s) {
            if (auto bus_statistics = catalogue.GetBusStatistics(name)) {
                response.emplace_back(MakeBusResponse(request_id, *bus_statistics));
            } else {
                response.emplace_back(MakeErrorResponse(request_id));
            }
        } else if (type == "Stop"s) {
            // TODO: make smart pointer here
            if (auto* buses = catalogue.GetBusesPassingThroughTheStop(name)) {
                response.emplace_back(MakeStopResponse(request_id, *buses));
            } else {
                response.emplace_back(MakeErrorResponse(request_id));
            }
        }
    }

    return response;
}

}  // namespace

void ProcessTransportCatalogueQuery(std::istream& input, std::ostream& output) {
    const auto input_json = json::Load(input).GetRoot();

    // Step 1. Form catalogue, basing on the input
    TransportCatalogue catalogue;
    const auto& base_requests = input_json.AsMap().at("base_requests").AsArray();
    auto transport_catalogue = ProcessBaseRequest(base_requests);

    // Step 2. Form response
    const auto& stat_requests = input_json.AsMap().at("stat_requests").AsArray();
    auto response = MakeStatResponse(transport_catalogue, stat_requests);

    json::Print(json::Document{std::move(response)}, output);
}

}  // namespace request