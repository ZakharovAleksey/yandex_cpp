#include "transport_catalogue.h"

#include <execution>
#include <numeric>

namespace catalogue {

void TransportCatalogue::AddStop(Stop stop) {
    UpdateMinMaxStopCoordinates(stop.point);

    // Add stop logic
    const auto position = stops_storage_.insert(stops_storage_.begin(), std::move(stop));
    stops_.insert({position->name, std::make_shared<Stop>(*position)});
    // Add stop for <stop-bus> correspondence
    buses_through_stop_.insert({position->name, {}});
}

void TransportCatalogue::AddDistance(std::string_view stop_from, std::string_view stop_to, int distance) {
    //! On this step we suppose that ALL stops have been parsed
    distances_between_stops_.insert({{stops_.at(stop_from), stops_.at(stop_to)}, distance});
}

void TransportCatalogue::AddBus(Bus bus) {
    //! On this step we suppose that ALL stops have been parsed
    for (auto& stop : bus.stop_names)
        stop = stops_.find(stop)->first;
    bus.unique_stops = {bus.stop_names.begin(), bus.stop_names.end()};

    const auto position = buses_storage_.insert(buses_storage_.begin(), std::move(bus));
    buses_.insert({position->number, std::make_shared<Bus>(*position)});
    ordered_bus_list.emplace(position->number);

    // Add stop for <stop-bus> correspondence
    for (std::string_view stop : position->stop_names)
        buses_through_stop_[stop].insert(position->number);
}

std::optional<BusStatistics> TransportCatalogue::GetBusStatistics(std::string_view bus_number) const {
    if (buses_.count(bus_number) == 0)
        return std::nullopt;

    auto bus_info = buses_.at(bus_number);

    BusStatistics result;
    result.number = bus_info->number;
    result.stops_count = bus_info->GetStopsCount();
    result.unique_stops_count = bus_info->unique_stops.size();
    result.rout_length = CalculateRouteLength(bus_info);
    result.curvature = static_cast<double>(result.rout_length) / CalculateGeographicLength(bus_info);

    return result;
}

int TransportCatalogue::CalculateRouteLength(const std::shared_ptr<Bus>& bus_info) const {
    auto get_route_length = [this](std::string_view from, std::string_view to) {
        auto key = std::make_pair(stops_.at(from), stops_.at(to));
        // If we not found 'from -> to' than we are looking for 'to -> from'
        return (distances_between_stops_.count(key) > 0)
                   ? distances_between_stops_.at(key)
                   : distances_between_stops_.at({stops_.at(to), stops_.at(from)});
    };

    int forward_route =
        std::transform_reduce(bus_info->stop_names.begin(), std::prev(bus_info->stop_names.end()),
                              std::next(bus_info->stop_names.begin()), 0, std::plus<>(), get_route_length);
    if (bus_info->type == RouteType::CIRCLE)
        return forward_route;

    // Otherwise, this is a two-directional way, so we need to calculate the distance on backward way

    int backward_route =
        std::transform_reduce(bus_info->stop_names.rbegin(), std::prev(bus_info->stop_names.rend()),
                              std::next(bus_info->stop_names.rbegin()), 0, std::plus<>(), get_route_length);

    return forward_route + backward_route;
}

double TransportCatalogue::CalculateGeographicLength(const std::shared_ptr<Bus>& bus_info) const {
    double geographic_length = std::transform_reduce(
        std::next(bus_info->stop_names.begin()), bus_info->stop_names.end(), bus_info->stop_names.begin(), 0.,
        std::plus<>(), [this](std::string_view from, std::string_view to) {
            return ComputeDistance(stops_.at(from)->point, stops_.at(to)->point);
        });

    return (bus_info->type == RouteType::CIRCLE) ? geographic_length : geographic_length * 2.;
}

void TransportCatalogue::UpdateMinMaxStopCoordinates(const geo::Coordinates& coordinates) {
    coordinates_min_.lat = std::min(coordinates_min_.lat, coordinates.lat);
    coordinates_min_.lng = std::min(coordinates_min_.lng, coordinates.lng);

    coordinates_max_.lat = std::max(coordinates_max_.lat, coordinates.lat);
    coordinates_max_.lng = std::max(coordinates_max_.lng, coordinates.lng);
}

const geo::Coordinates& TransportCatalogue::GetMinStopCoordinates() const {
    return coordinates_min_;
}

const geo::Coordinates& TransportCatalogue::GetMaxStopCoordinates() const {
    return coordinates_max_;
}

const std::set<std::string_view>& TransportCatalogue::GetOrderedBusList() const {
    return ordered_bus_list;
}

const std::set<std::string_view>* TransportCatalogue::GetBusesPassingThroughTheStop(std::string_view stop_name) const {
    if (const auto position = buses_through_stop_.find(stop_name); position != buses_through_stop_.cend())
        return &position->second;
    return nullptr;
}

}  // namespace catalogue