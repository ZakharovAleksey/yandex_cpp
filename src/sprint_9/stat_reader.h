#pragma once
#include <string>

#include "transport_catalogue.h"

namespace catalog::output_utils {
std::string_view ParseBusStatisticsRequest(std::string_view text);

std::string_view ParseBusesPassingThroughStopRequest(std::string_view text);

void PrintBusesPassingThroughStop(std::ostream& os, std::string_view stop_name,
                                  const std::set<std::string_view>* buses);

}  // namespace catalog::output_utils