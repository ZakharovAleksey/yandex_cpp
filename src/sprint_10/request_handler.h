#pragma once

/*
 * Description: module for processing requests.
 * Acts as a Facade that simplifies interaction with the transport directory
 */

#include "domain.h"

namespace request::utils {

/* Requests for input */

catalogue::DistancesToStops ParsePredefinedDistancesBetweenStops(std::string_view text);

std::pair<catalogue::Stop, bool> ParseBusStopInput(const std::string& text);

catalogue::Bus ParseBusRouteInput(std::string_view text);

/* Requests for output */

std::string_view ParseBusStatisticsRequest(std::string_view text);

std::string_view ParseBusesPassingThroughStopRequest(std::string_view text);

}  // namespace request::utils