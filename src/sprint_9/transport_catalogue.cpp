//
// Created by azakharov on 6/17/2021.
//

#include "transport_catalogue.h"

#include <execution>
#include <numeric>

namespace catalog {

size_t Bus::GetStopsCount() const {
    return (type == RouteType::CIRCLE) ? stop_names.size() : 2 * stop_names.size() - 1;
}

void TransportCatalogue::AddStop(Stop stop) {
    const auto position = stops_storage_.insert(stops_storage_.begin(), std::move(stop));
    stops_.insert({position->name, &(*position)});
    // Add stop for <stop-bus> correspondence
    buses_through_stop_.insert({position->name, {}});
}

void TransportCatalogue::AddDistancesBetweenStops(std::string_view stop_from, const DistancesToStops& distances) {
    //! On this step we suppose that ALL stops have been parsed
    std::string_view local_stop_from = stops_.find(stop_from)->first;

    for (const auto& [stop_to, distance] : distances) {
        StringViewPair key = std::make_pair(local_stop_from, stops_.find(stop_to)->first);
        distances_between_stops_.insert({std::move(key), distance});
    }
}

void TransportCatalogue::AddBus(Bus bus) {
    //! On this step we suppose that ALL stops have been parsed
    for (auto& stop : bus.stop_names)
        stop = stops_.find(stop)->first;
    bus.unique_stops = {bus.stop_names.begin(), bus.stop_names.end()};

    const auto position = buses_storage_.insert(buses_storage_.begin(), std::move(bus));
    buses_.insert({position->number, &(*position)});

    // Add stop for <stop-bus> correspondence
    for (std::string_view stop : position->stop_names)
        buses_through_stop_[stop].insert(position->number);
}

std::optional<BusStatistics> TransportCatalogue::GetBusStatistics(std::string_view bus_number) const {
    if (buses_.count(bus_number) == 0)
        return std::nullopt;

    const auto& bus_info = buses_.at(bus_number);

    BusStatistics result;
    result.number = bus_info->number;
    result.stops_count = bus_info->GetStopsCount();
    result.unique_stops_count = bus_info->unique_stops.size();
    result.rout_length = CalculateRouteLength(bus_info);
    result.curvature = result.rout_length / CalculateGeographicLength(bus_info);

    return result;
}

double TransportCatalogue::CalculateRouteLength(const Bus* bus_info) const {
    auto get_route_length = [this](std::string_view from, std::string_view to) {
        auto key = std::make_pair(from, to);
        // If we not found 'from -> to' than we are looking for 'to -> from'
        return (distances_between_stops_.count(key) > 0) ? distances_between_stops_.at(key)
                                                         : distances_between_stops_.at(std::make_pair(to, from));
    };

    double forward_route = std::transform_reduce(std::next(bus_info->stop_names.begin()), bus_info->stop_names.end(),
                                                 bus_info->stop_names.begin(), 0., std::plus<>(), get_route_length);
    if (bus_info->type == RouteType::CIRCLE)
        return forward_route;
    // Otherwise, this is a two-directional way, so we need to calculate the distance on backward way
    double backward_route = std::transform_reduce(std::next(bus_info->stop_names.rbegin()), bus_info->stop_names.rend(),
                                                  bus_info->stop_names.rbegin(), 0., std::plus<>(), get_route_length);
    return forward_route + backward_route;
}

double TransportCatalogue::CalculateGeographicLength(const Bus* bus_info) const {
    auto geographic_length = std::transform_reduce(
        std::next(bus_info->stop_names.begin()), bus_info->stop_names.end(), bus_info->stop_names.begin(), 0.,
        std::plus<>(), [this](std::string_view from, std::string_view to) {
            return ComputeDistance(stops_.at(from)->point, stops_.at(to)->point);
        });

    return (bus_info->type == RouteType::CIRCLE) ? geographic_length : geographic_length * 2.;
}

std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesPassingThroughTheStop(
    std::string_view stop_name) const {
    if (const auto position = buses_through_stop_.find(stop_name); position != buses_through_stop_.cend())
        return {position->second};
    return std::nullopt;
}

}  // namespace catalog