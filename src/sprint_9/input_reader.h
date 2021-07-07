#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"

/// данные функции являются частью каталога, поэтому пространство имен input_utils следует сделать подпростанством в catalog
/// можно использовать модный синтаксис: namespace catalog::input_utils
/// такое же замечание в output_utils не буду дублировать
namespace input_utils {

DistancesToStops ParsePredefinedDistancesBetweenStops(std::string_view text);

std::pair<catalog::Stop, bool> ParseBusStopInput(const std::string& text);

catalog::Bus ParseBusRouteInput(std::string_view text);

}  // namespace input_utils