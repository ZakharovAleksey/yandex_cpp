#include "json_reader.h"

#include <string>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace request {

using namespace catalogue;
using namespace request::utils;

using namespace std::literals;

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

render::Screen ParseScreenSettings(const json::Dict& settings) {
    render::Screen screen;

    screen.width_ = settings.at("width"s).AsDouble();
    screen.height_ = settings.at("height"s).AsDouble();
    screen.padding_ = settings.at("padding"s).AsDouble();

    return screen;
}

render::Label ParseLabelSettings(const json::Dict& settings, const std::string& key_type) {
    int font_size = settings.at(key_type + "_label_font_size"s).AsInt();
    const json::Array offset = settings.at(key_type + "_label_offset"s).AsArray();

    double offset_x = offset.at(0).AsDouble();
    double offset_y = offset.at(1).AsDouble();

    return {font_size, {offset_x, offset_y}};
}

svg::Color ParseColor(const json::Node& node) {
    // Node with color could be: string, rgb, rgba
    if (node.IsString())
        return node.AsString();

    const auto& array = node.AsArray();
    uint8_t red = array.at(0).AsInt();
    uint8_t green = array.at(1).AsInt();
    uint8_t blue = array.at(2).AsInt();

    // In case there is only 3 colors in the array - it is egb
    if (array.size() == 3)
        return svg::Rgb(red, green, blue);

    // Otherwise - this is rgba
    double alpha = array.at(3).AsDouble();
    return svg::Rgba(red, green, blue, alpha);
}

render::UnderLayer ParseLayer(const json::Dict& settings) {
    render::UnderLayer layer;

    layer.color_ = ParseColor(settings.at("underlayer_color"s));
    layer.width_ = settings.at("underlayer_width"s).AsInt();

    return layer;
}

render::Visualization ParseVisualizationSettings(const json::Dict& settings) {
    render::Visualization final_settings;

    double line_width = settings.at("line_width"s).AsDouble();
    double stop_radius = settings.at("stop_radius"s).AsDouble();

    // Parse list of colors
    const auto& colors = settings.at("color_palette"s).AsArray();
    std::vector<svg::Color> svg_colors;
    svg_colors.reserve(colors.size());
    for (const auto& color : colors)
        svg_colors.emplace_back(ParseColor(color));

    final_settings.SetScreen(ParseScreenSettings(settings))
        .SetLineWidth(line_width)
        .SetStopRadius(stop_radius)
        .SetLabels(render::LabelType::Stop, ParseLabelSettings(settings, "stop"s))
        .SetLabels(render::LabelType::Bus, ParseLabelSettings(settings, "bus"s))
        .SetUnderLayer(ParseLayer(settings))
        .SetColors(std::move(svg_colors));

    return final_settings;
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

json::Node MakeMapImageResponse(int request_id, const std::string& image) {
    json::Dict response;

    response.emplace("request_id"s, request_id);
    response.emplace("map"s, image);

    return response;
}

json::Node MakeStatResponse(const TransportCatalogue& catalogue, const json::Array& requests,
                            const render::Visualization& settings) {
    json::Array response;
    response.reserve(requests.size());

    for (const auto& request : requests) {
        const auto& request_dict_view = request.AsMap();

        int request_id = request_dict_view.at("id"s).AsInt();
        std::string type = request_dict_view.at("type"s).AsString();
        std::string name;  //> Could be a name of bus or a stop

        if (type == "Bus"s) {
            name = request_dict_view.at("name"s).AsString();

            if (auto bus_statistics = catalogue.GetBusStatistics(name)) {
                response.emplace_back(MakeBusResponse(request_id, *bus_statistics));
            } else {
                response.emplace_back(MakeErrorResponse(request_id));
            }
        } else if (type == "Stop"s) {
            name = request_dict_view.at("name"s).AsString();
            // TODO: make smart pointer here
            if (auto* buses = catalogue.GetBusesPassingThroughTheStop(name)) {
                response.emplace_back(MakeStopResponse(request_id, *buses));
            } else {
                response.emplace_back(MakeErrorResponse(request_id));
            }
        } else if (type == "Map"s) {
            std::string image = RenderTransportMap(catalogue, settings);
            response.emplace_back(MakeMapImageResponse(request_id, image));
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

    // Step 2. Parse rendering settings
    const auto& render_settings = input_json.AsMap().at("render_settings").AsMap();
    const auto& visualization_settings = ParseVisualizationSettings(render_settings);

    // Step 3. Form response
    const auto& stat_requests = input_json.AsMap().at("stat_requests").AsArray();
    auto response = MakeStatResponse(transport_catalogue, stat_requests, visualization_settings);

    json::Print(json::Document{std::move(response)}, output);
}

}  // namespace request