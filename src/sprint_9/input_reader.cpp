//
// Created by azakharov on 6/17/2021.
//

#include "input_reader.h"

#include <cassert>
#include <regex>

namespace input_utils {

using namespace std::literals;
using namespace catalog;

constexpr int kMaxStopsCount{100};

DistancesToStops ParsePredefinedDistancesBetweenStops(std::string_view text) {
    //! Input format: Stop X: latitude, longitude, D1m to stop1, D2m to stop2, ...

    const std::regex distance_to_stop_pattern(", ([0-9]+)m to ([a-zA-Z ]+) ?");

    DistancesToStops result;
    result.reserve(kMaxStopsCount);

    std::match_results<std::string_view::const_iterator> match;
    int offset{0};
    while (std::regex_search(text.begin() + offset, text.end(), match, distance_to_stop_pattern)) {
        int stop_offset = std::distance(text.cbegin(), match[2].first);  // offset + 2 + match[1].length() + 5;
        result.emplace_back(text.substr(stop_offset, match[2].length()), std::stoi(match[1]));
        offset += match.length();
    }

    return result;
}

std::pair<catalog::Stop, bool> ParseBusStopInput(std::string_view text) {
    //! Input format without stops info: Stop X: latitude, longitude
    //! Input format with stops info: Stop X: latitude, longitude, D1m to stop1, D2m to stop2, ...

    const std::regex pattern("^Stop (.+): ([-]?[0-9]+[.]?[0-9]*), ([-]?[0-9]+[.]?[0-9]*)");
    std::match_results<std::string_view::const_iterator> match;

    // Search one time for the first information about bus
    assert(std::regex_search(text.cbegin(), text.cend(), match, pattern) &&
           "Stop info input does not match the pattern");

    bool has_stops_info = text.size() != match.length();
    return {{match[1], {std::stod(match[2]), std::stod(match[3])}}, has_stops_info};
}

Bus ParseBusRouteInput(std::string_view text) {
    //! Input format for circle route: Bus Y: Stop#1 > Stop#2 > Stop#3 ..
    //! Input format for two-directional route: Bus Y: Stop#1 - Stop#2 - Stop#3 ..

    Bus result;

    const std::regex bus_pattern("^Bus ([a-zA-Z0-9 ]+):");
    std::match_results<std::string_view::const_iterator> match;

    assert(std::regex_search(text.begin(), text.end(), match, bus_pattern) && "Input does not match bus pattern");
    result.number = match[1];

    result.type = text[text.find_first_of("->")] == '>' ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;

    const std::regex stop_pattern(" ([a-zA-Z ]+)(?= |$)");
    int offset = match.length();
    while (std::regex_search(text.begin() + offset, text.end(), match, stop_pattern)) {
        int stop_offset = std::distance(text.begin(), match[1].first);
        result.stop_names.emplace_back(text.substr(stop_offset, match[1].length()));
        offset = stop_offset + match.length() - 1;
    }

    return result;
}

}  // namespace input_utils