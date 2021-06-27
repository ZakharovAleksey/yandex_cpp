#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"

namespace input_utils {

DistancesToStops ParsePredefinedDistancesBetweenStops(std::string_view text);

std::pair<catalog::Stop, bool> ParseBusStopInput(const std::string& text);

catalog::Bus ParseBusRouteInput(std::string_view text);

}  // namespace input_utils