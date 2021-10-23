#pragma once

/*
 * Description: module for processing requests.
 * Acts as a Facade that simplifies interaction with the transport directory
 */

#include "json_reader.h"

namespace request {

void ProcessTransportCatalogueQuery(std::istream& input, std::ostream& output);

}  // namespace request