#pragma once
#include <string>

#include "transport_catalogue.h"

namespace output_utils {
std::string ParseBusStatisticsRequest(std::string_view text);

std::string ParseBusesPassingThroughStopRequest(std::string_view text);

void PrintBusStatistics(std::ostream& os, std::string_view bus_number, std::optional<catalog::BusStatistics> bus_info);

void PrintBusesPassingThroughStop(std::ostream& os, std::string_view stop_name, std::optional<std::set<std::string_view>> buses);

}  // namespace output_utils