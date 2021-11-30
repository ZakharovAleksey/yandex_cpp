#pragma once

#include <filesystem>

#include "transport_catalogue.h"

namespace serialization {
using Path = std::filesystem::path;

bool SerializeTransportCatalogue(const Path& storage_path, const catalogue::TransportCatalogue& catalogue);

catalogue::TransportCatalogue DeserializeTransportCatalogue(const Path& load_path);

}  // namespace serialization