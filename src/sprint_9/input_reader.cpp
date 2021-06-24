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
    using svregex_iterator = std::regex_iterator<std::string_view::const_iterator>;
    using sv_match = std::match_results<std::string_view::const_iterator>;

    const std::regex distance_to_stop_pattern("(, ([0-9]+)m to ([a-zA-Z ]+) ?)");

    DistancesToStops result;

    auto patterns_match_begin = svregex_iterator(text.begin(), text.end(), distance_to_stop_pattern);
    auto patterns_match_end = svregex_iterator();

    size_t matched_patterns_count = std::distance(patterns_match_begin, patterns_match_end);
    if (matched_patterns_count != 0u) {
        result.reserve(matched_patterns_count);
        sv_match match;
        // Loop over all matched patterns, extracting information: stop_name & distance
        for (svregex_iterator i = patterns_match_begin; i != patterns_match_end; ++i) {
            std::string_view distance_to_stop = text.substr(i->position(), i->length());
            std::regex_match(distance_to_stop.cbegin(), distance_to_stop.cend(), match, distance_to_stop_pattern);

            result.emplace_back(match[3], std::stod(match[2]));
        }
    }

    return result;
}

std::pair<catalog::Stop, DistancesToStops> ParseBusStopInput(std::string_view text) {
    std::regex pattern("^Stop (.+): ([0-9]+[.][0-9]+), ([0-9]+[.][0-9]+)(.+?)$");
    std::match_results<std::string_view::const_iterator> match;

    assert(std::regex_match(text.cbegin(), text.cend(), match, pattern) &&
           "Stop info input does not match the pattern");

    return {{match[1], {std::stod(match[2]), std::stod(match[3])}}, ParsePredefinedDistancesBetweenStops(text)};
}

Bus ParseBusRouteInput(std::string_view text) {
    const size_t bus_index_start{4u};
    const std::string bus_stops_delimiter{": "};

    Bus result;

    size_t bus_index_end = text.find(bus_stops_delimiter);
    size_t stops_offset = bus_index_end + bus_stops_delimiter.size();

    result.number = text.substr(bus_index_start, bus_index_end - bus_index_start);

    char delimiter = text[text.find_first_of("->")];
    result.type = delimiter == '>' ? RouteType::CIRCLE : RouteType::TWO_DIRECTIONAL;

    std::string stops_separator = " " + std::string(1, delimiter) + " ";
    size_t stop_begin = stops_offset;
    while (stop_begin <= text.length()) {
        size_t word_end = text.find(stops_separator, stop_begin);

        result.stop_names.push_back(text.substr(stop_begin, word_end - stop_begin));
        stop_begin = (word_end == std::string_view::npos) ? word_end : word_end + stops_separator.size();
    }

    result.unique_stops = {result.stop_names.begin(), result.stop_names.end()};

    return result;
}

}  // namespace input_utils