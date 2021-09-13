//
// Created by azakharov on 6/17/2021.
//

#include "input_reader.h"
#include "stat_reader.h"

#include <cassert>
#include <regex>


namespace catalog::input_utils {

using namespace std::literals;
using namespace catalog;
using namespace output_utils;

DistancesToStops ParsePredefinedDistancesBetweenStops(std::string_view text) {
    //! Input format: Stop X: latitude, longitude, D1m to stop1, D2m to stop2, ...

    DistancesToStops result;

    // Looking for the second ',' in a row
    size_t start = text.find(',');
    start = text.find(',', start + 1) + (" "sv).size();
    size_t end = start;

    while (start != std::string_view::npos) {
        end = text.find("m"sv, start);
        int distance = std::stoi(std::string(text.substr(start, end - start)));

        start = end + ("m to "sv).size();
        end = text.find(","sv, start);

        std::string_view stop_to = text.substr(start, end - start);
        result.emplace_back(stop_to, distance);

        start = (end == std::string_view::npos) ? end : end + (" "sv).size();
    }

    return result;
}

std::pair<catalog::Stop, bool> ParseBusStopInput(const std::string& text) {
    //! Input format without stops info: Stop X: latitude, longitude
    //! Input format with stops info: Stop X: latitude, longitude, D1m to stop1, D2m to stop2, ...

    Stop stop;

    size_t stop_begin = ("Stop "s).size();
    size_t stop_end = text.find(": "s, stop_begin);
    stop.name = text.substr(stop_begin, stop_end - stop_begin);

    size_t latitude_begin = stop_end + (": "s).size();
    size_t latitude_end = text.find(","s, latitude_begin);
    stop.point.lat = std::stod(text.substr(latitude_begin, latitude_end - latitude_begin));

    size_t longitude_begin = latitude_end + (", "s).size();
    size_t longitude_end = text.find(","s, longitude_begin);
    stop.point.lng = std::stod(text.substr(longitude_begin, longitude_end - longitude_begin));

    bool has_stops_info = longitude_end != std::string_view::npos;
    return {std::move(stop), has_stops_info};
}

Bus ParseBusRouteInput(std::string_view text) {
    //! Input format for circle route: Bus Y: Stop#1 > Stop#2 > Stop#3 ..
    //! Input format for two-directional route: Bus Y: Stop#1 - Stop#2 - Stop#3 ..

    Bus result;

    size_t bus_start = text.find(' ') + (" "sv).size();
    size_t bus_end = text.find(": "sv, bus_start);
    result.number = text.substr(bus_start, bus_end - bus_start);

    result.type = (text[text.find_first_of("->")] == '>') ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;
    std::string_view stops_separator = (result.type == RouteType::CIRCLE) ? " > "sv : " - "sv;

    size_t stop_begin = bus_end + (": "sv).size();
    while (stop_begin <= text.length()) {
        size_t stop_end = text.find(stops_separator, stop_begin);

        result.stop_names.push_back(text.substr(stop_begin, stop_end - stop_begin));
        stop_begin = (stop_end == std::string_view::npos) ? stop_end : stop_end + stops_separator.size();
    }

    result.unique_stops = {result.stop_names.begin(), result.stop_names.end()};

    return result;
}

void ParseTransportCatalogueQueries(std::istream& input_stream) {
    TransportCatalogue catalogue;

    int queries_count{0};
    input_stream >> queries_count;
    input_stream.get();

    std::vector<std::string> bus_queries;
    bus_queries.reserve(queries_count);
    std::vector<std::pair<std::string, std::string>> stop_distances;
    stop_distances.reserve(queries_count);

    std::string query;
    for (int id = 0; id < queries_count; ++id) {
        std::getline(input_stream, query);
        if (query.substr(0, 4) == "Stop"s) {
            auto [stop, is_store_query] = ParseBusStopInput(query);
            if (is_store_query)
                stop_distances.emplace_back(stop.name, std::move(query));
            catalogue.AddStop(std::move(stop));
        } else if (query.substr(0, 3) == "Bus"s) {
            bus_queries.emplace_back(std::move(query));
        }
    }

    for (const auto& [stop_from, query] : stop_distances) {
        for (auto [stop_to, distance] : ParsePredefinedDistancesBetweenStops(query))
            catalogue.AddDistance(stop_from, stop_to, distance);
    }

    for (const auto& bus_query : bus_queries)
        catalogue.AddBus(ParseBusRouteInput(bus_query));

    input_stream >> queries_count;
    input_stream.get();
    for (int id = 0; id < queries_count; ++id) {
        std::getline(input_stream, query);
        if (query.substr(0, 3) == "Bus"s) {
            std::string_view bus_number = ParseBusStatisticsRequest(query);

            if (auto bus_statistics = catalogue.GetBusStatistics(bus_number)) {
                std::cout << *bus_statistics << std::endl;
            } else {
                std::cout << "Bus " << bus_number << ": not found" << std::endl;
            }
        } else if (query.substr(0, 4) == "Stop"s) {
            std::string_view stop_name = ParseBusesPassingThroughStopRequest(query);
            auto* buses = catalogue.GetBusesPassingThroughTheStop(stop_name);

            PrintBusesPassingThroughStop(std::cout, stop_name, buses);
        }
    }
}

}  // namespace catalog::input_utils