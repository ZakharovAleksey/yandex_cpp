#include "serialization.h"

namespace serialization {

bool SerializeTransportCatalogue(const Path& storage_path, const catalogue::TransportCatalogue& catalogue) {
    return true;
}

catalogue::TransportCatalogue DeserializeTransportCatalogue(const Path& load_path) {
    return {};
}

}  // namespace serialization
