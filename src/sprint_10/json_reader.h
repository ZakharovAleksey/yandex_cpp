#pragma once

/*
 * Description: parses JSON data built during parsing and forms an array
 * of JSON responses
 */

#include <iostream>
#include <set>
#include <string_view>

namespace request {

void PrintBusesPassingThroughStop(std::ostream& os, std::string_view stop_name,
                                  const std::set<std::string_view>* buses);

void ParseTransportCatalogueQueries(std::istream& input_stream);

}  // namespace request