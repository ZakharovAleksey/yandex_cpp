#pragma once

/*
 * Description: module for processing requests.
 * Acts as a Facade that simplifies interaction with the transport directory
 */

#include "domain.h"
#include "json.h"

namespace request::utils {

std::pair<catalogue::Stop, bool> ParseBusStopInput(const json::Dict& info);

catalogue::Bus ParseBusRouteInput(const json::Dict& info);

}  // namespace request::utils