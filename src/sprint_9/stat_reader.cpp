//
// Created by azakharov on 6/17/2021.
//

#include "stat_reader.h"

#include <cassert>
#include <iomanip>
#include <regex>

namespace output_utils {
// TODO: string_view instead of string
std::string ParseBusStatisticsRequest(std::string_view text) {
    std::regex pattern("^Bus (.+)$");
    std::match_results<std::string_view::const_iterator> match;

    assert(std::regex_match(text.cbegin(), text.cend(), match, pattern) &&
           "Bus statistics request pattern does not match");
    return match[1];
}

void PrintBusStatistics(std::ostream& os, std::string bus_number, std::optional<catalog::BusStatistics> bus_info) {
    if (!bus_info) {
        os << "Bus " << bus_number << ": not found" << std::endl;
    } else {
        os << "Bus " << bus_info->number << ": " << bus_info->stops_count << " stops on route, "
           << bus_info->unique_stops_count << " unique stops, ";
        os << std::setprecision(6) << bus_info->rout_length << " route length" << std::endl;
    }
}

}  // namespace output_utils