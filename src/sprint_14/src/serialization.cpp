#include "serialization.h"

#include <map_renderer.pb.h>
#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>

#include <fstream>

namespace serialization {

using StopNameToIdContaner = std::unordered_map<std::string_view, int>;
// TODO: maybe string_view here
using IdToStopNameContainer = std::unordered_map<int, std::string>;

namespace {
StopNameToIdContaner SetIdToEachStop(const std::deque<catalogue::Stop>& stops) {
    // !!! IMPORTANT !!! Store IDs in the direct order for SERIALIZATION
    StopNameToIdContaner result;
    result.reserve(stops.size());

    for (int id = 0; id < stops.size(); ++id)
        result.emplace(stops[id].name, id);

    return result;
}

IdToStopNameContainer SetNameToEachStop(const std::deque<catalogue::Stop>& stops) {
    // !!! IMPORTANT !!! Store IDs in the reverse order for DESERIALIZATION
    IdToStopNameContainer result;
    result.reserve(stops.size());

    for (int id = 0; id < stops.size(); ++id)
        result.emplace(id, stops[stops.size() - id - 1].name);

    return result;
}

}  // namespace

void SerializeTransportCatalogue(std::ofstream& output, const catalogue::TransportCatalogue& catalogue) {
    proto_tc::TransportCatalogue object;

    const auto& stops = catalogue.GetStops();
    const auto stop_to_id = SetIdToEachStop(stops);

    // Step 1. Serialize stops
    for (const auto& stop : stops) {
        proto_tc::Stop stop_object;

        stop_object.set_name(stop.name);
        stop_object.mutable_point()->set_lng(stop.point.lng);
        stop_object.mutable_point()->set_lat(stop.point.lat);

        object.mutable_stops()->Add(std::move(stop_object));
    }

    // Step 2. Serialize distances between stops
    const auto& distances = catalogue.GetDistancesBetweenStops();
    for (const auto& [stops_pair, distance] : distances) {
        proto_tc::DistanceBetweenStops distance_object;

        distance_object.set_from(stop_to_id.at(stops_pair.first->name));
        distance_object.set_to(stop_to_id.at(stops_pair.second->name));
        distance_object.set_distance(distance);

        object.mutable_distances()->Add(std::move(distance_object));
    }

    // Step 3. Serialize buses
    const auto& buses = catalogue.GetBuses();
    for (const auto& bus : buses) {
        proto_tc::Bus bus_object;

        bus_object.set_name(bus.number);
        bus_object.set_is_circle(bus.type == catalogue::RouteType::CIRCLE);
        for (std::string_view stop : bus.stop_names)
            bus_object.add_stops_ids(stop_to_id.at(stop));

        object.mutable_buses()->Add(std::move(bus_object));
    }

    object.SerializeToOstream(&output);
}

catalogue::TransportCatalogue DeserializeTransportCatalogue(const catalogue::Path& path) {
    proto_tc::TransportCatalogue object;
    catalogue::TransportCatalogue catalogue;

    std::ifstream input(path, std::ios::binary);
    object.ParseFromIstream(&input);

    auto to_int = [](uint32_t value) { return static_cast<int>(value); };

    // Step 1. Parse stops
    for (const auto& stop_object : object.stops()) {
        catalogue::Stop stop;

        stop.name = stop_object.name();
        stop.point.lng = stop_object.point().lng();
        stop.point.lat = stop_object.point().lat();

        catalogue.AddStop(std::move(stop));
    }

    const auto id_to_stop = SetNameToEachStop(catalogue.GetStops());

    // Step 2. Parse all distances between stops
    for (const auto& distance_object : object.distances()) {
        std::string_view from = id_to_stop.at(to_int(distance_object.from()));
        std::string_view to = id_to_stop.at(to_int(distance_object.to()));

        catalogue.AddDistance(from, to, to_int(distance_object.distance()));
    }

    // Step 3. Parse all buses
    using Route = catalogue::RouteType;
    for (const auto& bus_object : object.buses()) {
        catalogue::Bus bus;

        bus.number = bus_object.name();
        bus.type = bus_object.is_circle() ? Route::CIRCLE : Route::TWO_DIRECTIONAL;

        bus.stop_names.reserve(bus_object.stops_ids_size());
        for (uint32_t stop_id : bus_object.stops_ids())
            bus.stop_names.emplace_back(id_to_stop.at(to_int(stop_id)));

        catalogue.AddBus(std::move(bus));
    }

    return catalogue;
}

void SerializeVisualizationSettings(std::ofstream& output, const render::Visualization& settings) {
    proto_render::MapRenderer object;

    auto set_color = [](const svg::Color& color) {
        // Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
        proto_render::Color object;

        if (std::holds_alternative<std::monostate>(color)) {
            object.set_is_none(true);
        } else if (std::holds_alternative<std::string>(color)) {
            object.set_name(std::get<std::string>(color));
        } else {
            bool is_rgba = std::holds_alternative<svg::Rgba>(color);
            object.mutable_rgba()->set_is_rgba(is_rgba);

            if (is_rgba) {
                svg::Rgba rgba = std::get<svg::Rgba>(color);
                object.mutable_rgba()->set_red(rgba.red);
                object.mutable_rgba()->set_green(rgba.green);
                object.mutable_rgba()->set_blue(rgba.blue);
                object.mutable_rgba()->set_opacity(rgba.opacity);
            } else {
                svg::Rgb rgb = std::get<svg::Rgb>(color);
                object.mutable_rgba()->set_red(rgb.red);
                object.mutable_rgba()->set_green(rgb.green);
                object.mutable_rgba()->set_blue(rgb.blue);
            }
        }

        return object;
    };

    const auto& screen = settings.GetScreen();
    object.mutable_screen()->set_width(screen.width_);
    object.mutable_screen()->set_height(screen.height_);
    object.mutable_screen()->set_padding(screen.padding_);

    object.set_stop_radius(settings.GetStopRadius());
    object.set_line_width(settings.GetLineWidth());

    const auto& bus = settings.GetLabels(render::LabelType::Bus);
    object.mutable_bus()->set_font_size(bus.font_size_);
    object.mutable_bus()->mutable_offset()->set_x(bus.offset_.x);
    object.mutable_bus()->mutable_offset()->set_y(bus.offset_.y);

    const auto& stop = settings.GetLabels(render::LabelType::Stop);
    object.mutable_stop()->set_font_size(stop.font_size_);
    object.mutable_stop()->mutable_offset()->set_x(stop.offset_.x);
    object.mutable_stop()->mutable_offset()->set_y(stop.offset_.y);

    const auto& background = settings.GetUnderLayer();
    object.mutable_background()->set_width(background.width_);
    *object.mutable_background()->mutable_color() = set_color(background.color_);

    for (const auto& color : settings.GetColors())
        object.mutable_color_palette()->Add(set_color(color));

    object.SerializeToOstream(&output);
}

render::Visualization DeserializeVisualizationSettings(const catalogue::Path& path) {
    auto set_color = [](const proto_render::Color& object) -> svg::Color {
        if (object.is_none()) {
            return std::monostate();
        } else if (!object.name().empty()) {
            return object.name();
        } else {
            bool is_rgba = object.rgba().is_rgba();
            if (is_rgba) {
                svg::Rgba color;
                color.red = object.rgba().red();
                color.green = object.rgba().green();
                color.blue = object.rgba().blue();
                color.opacity = object.rgba().opacity();
                return color;
            } else {
                svg::Rgb color;
                color.red = object.rgba().red();
                color.green = object.rgba().green();
                color.blue = object.rgba().blue();
                return color;
            }
        }
    };

    proto_render::MapRenderer object;
    std::ifstream input(path, std::ios::binary);
    object.ParseFromIstream(&input);

    render::Visualization settings;

    render::Screen screen;
    screen.width_ = object.screen().width();
    screen.height_ = object.screen().height();
    screen.padding_ = object.screen().padding();
    settings.SetScreen(screen);

    settings.SetStopRadius(object.stop_radius()).SetLineWidth(object.line_width());

    render::Label bus;
    bus.font_size_ = object.bus().font_size();
    bus.offset_.x = object.bus().offset().x();
    bus.offset_.y = object.bus().offset().y();
    settings.SetLabels(render::LabelType::Bus, bus);

    render::Label stop;
    stop.font_size_ = object.stop().font_size();
    stop.offset_.x = object.stop().offset().x();
    stop.offset_.y = object.stop().offset().y();
    settings.SetLabels(render::LabelType::Stop, stop);

    render::UnderLayer background;
    background.width_ = object.background().width();
    background.color_ = set_color(object.background().color());
    settings.SetUnderLayer(background);

    std::vector<svg::Color> svg_colors;
    svg_colors.reserve(object.color_palette_size());
    for (const auto& color : object.color_palette())
        svg_colors.emplace_back(set_color(color));
    settings.SetColors(std::move(svg_colors));

    return settings;
}

void SerializeRoutingSettings(std::ofstream& output, const routing::Settings& settings) {
    proto_router::Settings object;

    object.set_bus_velocity(settings.bus_velocity_);
    object.set_bus_wait_time(settings.bus_wait_time_);

    object.SerializeToOstream(&output);
}

routing::Settings DeserializeRoutingSettings(const catalogue::Path& path) {
    std::ifstream input(path, std::ios::binary);
    proto_router::Settings object;
    object.ParseFromIstream(&input);

    routing::Settings settings;
    settings.bus_velocity_ = object.bus_velocity();
    settings.bus_wait_time_ = static_cast<int>(object.bus_wait_time());

    return settings;
}

}  // namespace serialization
