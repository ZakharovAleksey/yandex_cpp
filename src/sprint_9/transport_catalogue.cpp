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
}

void TransportCatalogue::AddBus(Bus bus) {
    //! On this step we suppose that ALL stops have been parsed
    for (auto& stop : bus.stop_names)
        stop = stops_.find(stop)->first;
    bus.unique_stops = {bus.stop_names.begin(), bus.stop_names.end()};

    const auto position = buses_storage_.insert(buses_storage_.begin(), std::move(bus));
    buses_.insert({position->number, &(*position)});
}

std::optional<BusStatistics> TransportCatalogue::GetBusStatistics(std::string_view bus_number) const {
    if (buses_.count(bus_number) == 0)
        return std::nullopt;

    const auto& bus_info = buses_.at(bus_number);

    BusStatistics result;
    result.number = bus_info->number;
    result.stops_count = bus_info->GetStopsCount();
    result.unique_stops_count = bus_info->unique_stops.size();

    result.rout_length = std::transform_reduce(std::next(bus_info->stop_names.begin()), bus_info->stop_names.end(),
                                               bus_info->stop_names.begin(), 0., std::plus<>(),
                                               [this](std::string_view from, std::string_view to) {
                                                   return ComputeDistance(stops_.at(from)->point, stops_.at(to)->point);
                                               });
    result.rout_length = (bus_info->type == RouteType::CIRCLE) ? result.rout_length : result.rout_length * 2.;

    return result;
}
}  // namespace catalog