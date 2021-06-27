//
// Created by azakharov on 6/17/2021.
//

#include "input_reader.h"

#include <cassert>
#include <regex>

namespace input_utils {

using namespace std::literals;
using namespace catalog;

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

}  // namespace input_utils