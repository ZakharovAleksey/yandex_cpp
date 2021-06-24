#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"


namespace input_utils {

std::pair<catalog::Stop, DistancesToStops> ParseBusStopInput(std::string_view text);

catalog::Bus ParseBusRouteInput(std::string_view text);

}  // namespace input_utils