#pragma once

/*
 * Description: parses JSON data built during parsing and forms an array
 * of JSON responses
 */

#include <iostream>
#include <set>
#include <string_view>

namespace request {

void ProcessTransportCatalogueQuery(std::istream& input, std::ostream& output);

}  // namespace request