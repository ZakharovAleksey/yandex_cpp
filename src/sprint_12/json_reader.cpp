#include "json_reader.h"

#include <string>

#include "json_builder.h"

namespace request {

using namespace std::literals;
using namespace catalogue;
using namespace request;
using namespace routing;

namespace {

std::pair<catalogue::Stop, bool> ParseBusStopInput(const json::Dict& info) {
    Stop stop;

    stop.name = info.at("name"s).AsString();
    stop.point.lat = info.at("latitude"s).AsDouble();
    stop.point.lng = info.at("longitude"s).AsDouble();

    bool has_road_distances = !info.at("road_distances"s).AsDict().empty();

    return {std::move(stop), has_road_distances};
}

Bus ParseBusRouteInput(const json::Dict& info) {
    Bus bus;

    bus.number = info.at("name"s).AsString();
    bus.type = info.at("is_roundtrip"s).AsBool() ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;

    const auto& stops = info.at("stops"s).AsArray();
    bus.stop_names.reserve(stops.size());

    for (const auto& stop : stops)
        bus.stop_names.emplace_back(stop.AsString());

    bus.unique_stops = {bus.stop_names.begin(), bus.stop_names.end()};

    return bus;
}

void MakeBusResponse(int request_id, const BusStatistics& statistics, json::Builder& response) {
    // P.S. no need to use std::move() because all types on the right are trivial

    response.StartDict();
    response.Key("curvature"s).Value(statistics.curvature);
    response.Key("request_id"s).Value(request_id);
    response.Key("route_length"s).Value(statistics.rout_length);
    response.Key("stop_count"s).Value(static_cast<int>(statistics.stops_count));
    response.Key("unique_stop_count"s).Value(static_cast<int>(statistics.unique_stops_count));
    response.EndDict();
}

void MakeStopResponse(int request_id, const std::set<std::string_view>& buses, json::Builder& response) {
    response.StartDict();
    response.Key("request_id"s).Value(request_id);

    response.Key("buses"s).StartArray();
    for (std::string_view bus : buses)
        response.Value(std::string(bus));
    response.EndArray();

    response.EndDict();
}

void MakeRouteResponse(int request_id, const routing::ResponseData& route_info, json::Builder& response) {
    response.StartDict();

    response.Key("request_id"s).Value(request_id);
    response.Key("total_time"s).Value(route_info.total_time_);

    response.Key("items"s).StartArray();

    for (const auto& item : route_info.items_) {
        response.StartDict();
        response.Key("type"s).Value(item.type_);

        if (item.type_ == "Wait"s) {
            response.Key("stop_name"s).Value(*item.stop_name_);
        } else if (item.type_ == "Bus"s) {
            response.Key("bus").Value(*item.bus_);
            response.Key("span_count"s).Value(*item.span_count_);
        }

        response.Key("time"s).Value(item.time_);
        response.EndDict();
    }

    response.EndArray();

    response.EndDict();
}

void MakeErrorResponse(int request_id, json::Builder& response) {
    response.StartDict();
    response.Key("request_id"s).Value(request_id);
    response.Key("error_message"s).Value("not found"s);
    response.EndDict();
}

void MakeMapImageResponse(int request_id, const std::string& image, json::Builder& response) {
    response.StartDict();
    response.Key("request_id"s).Value(request_id);
    response.Key("map"s).Value(image);
    response.EndDict();
}

/* METHODS FOR MAP IMAGE RENDERING */

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
    layer.width_ = settings.at("underlayer_width"s).AsDouble();

    return layer;
}

TransportRouter MakeTransportRouter(const TransportCatalogue& catalogue, routing::Settings settings) {
    // clang-format off
    return TransportRouter(settings, TransportRouterInputData{
                                         .stops_ = catalogue.GetUniqueStops(),
                                         .buses_ = catalogue.GetBuses(),
                                         .distances_ = catalogue.GetInterStopsDistances()
                                     });
    // clang-format on
}

}  // namespace

TransportCatalogue ProcessBaseRequest(const json::Array& requests) {
    TransportCatalogue catalogue;

    // We could add distances between stops ONLY when they EXIST in catalogue
    std::vector<int> requests_ids_with_road_distances;
    requests_ids_with_road_distances.reserve(requests.size());

    std::vector<int> requests_ids_with_buses;
    requests_ids_with_buses.reserve(requests.size());

    // Step 1. Store all stops to the catalog and mark requests, needed to be processed afterward
    for (int id = 0; id != static_cast<int>(requests.size()); ++id) {
        const auto& request_dict_view = requests.at(id).AsDict();

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
        const auto& request_dict_view = requests.at(id).AsDict();

        std::string_view stop_from = request_dict_view.at("name"s).AsString();
        for (const auto& [stop_to, distance] : request_dict_view.at("road_distances"s).AsDict())
            catalogue.AddDistance(stop_from, stop_to, distance.AsInt());
    }

    // Step 3. Add info about buses routes through stops
    for (int id : requests_ids_with_buses) {
        const auto& request_dict_view = requests.at(id).AsDict();
        catalogue.AddBus(ParseBusRouteInput(request_dict_view));
    }

    return catalogue;
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

json::Node MakeStatResponse(const TransportCatalogue& catalogue, TransportRouterOpt& router,
                            const json::Array& requests, const ResponseSettings& settings) {
    auto response = json::Builder();
    response.StartArray();

    for (const auto& request : requests) {
        const auto& request_dict_view = request.AsDict();

        int request_id = request_dict_view.at("id"s).AsInt();
        std::string type = request_dict_view.at("type"s).AsString();
        std::string name;  //> Could be a name of bus or a stop

        if (type == "Bus"s) {
            name = request_dict_view.at("name"s).AsString();

            if (auto bus_statistics = catalogue.GetBusStatistics(name)) {
                MakeBusResponse(request_id, *bus_statistics, response);
            } else {
                MakeErrorResponse(request_id, response);
            }
        } else if (type == "Stop"s) {
            name = request_dict_view.at("name"s).AsString();
            if (auto buses = catalogue.GetBusesPassingThroughTheStop(name)) {
                MakeStopResponse(request_id, *buses, response);
            } else {
                MakeErrorResponse(request_id, response);
            }
        } else if (type == "Map"s) {
            std::string image = RenderTransportMap(catalogue, settings.visualization);
            MakeMapImageResponse(request_id, image, response);
        } else if (type == "Route"s) {
            // Create router if it is still empty - crete only once
            if (!router.has_value())
                router.emplace(MakeTransportRouter(catalogue, settings.routing));

            std::string stop_name_from = request_dict_view.at("from"s).AsString();
            std::string stop_name_to = request_dict_view.at("to"s).AsString();

            if (auto route_info = router->BuildRoute(stop_name_from, stop_name_to)) {
                MakeRouteResponse(request_id, *route_info, response);
            } else {
                MakeErrorResponse(request_id, response);
            }
        }
    }

    response.EndArray();
    return std::move(response.Build());
}

routing::Settings ParseRoutingSettings(const json::Dict& requests) {
    using namespace routing;

    // clang-format off
    Settings settings{
        .bus_velocity_ = requests.at("bus_velocity"s).AsInt(),
        .bus_wait_time_ = requests.at("bus_wait_time"s).AsInt()
    };
    // clang-format on

    return settings;
}

}  // namespace request