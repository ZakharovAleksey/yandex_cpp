#pragma once

#include "transport_catalogue.h"

namespace serialization {

void SerializeTransportCatalogue(const catalogue::Path& path, const catalogue::TransportCatalogue& catalogue);

catalogue::TransportCatalogue DeserializeTransportCatalogue(const catalogue::Path& path);

};  // namespace serialization
