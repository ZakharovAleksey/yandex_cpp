#pragma once
#include <string>

#include "transport_catalogue.h"

namespace output_utils {
std::string ParseBusStatisticsRequest(std::string_view text);

void PrintBusStatistics(std::ostream& os, std::string bus_number, std::optional<catalog::BusStatistics> bus_info);

}  // namespace output_utils