#include "domain.h"

#include <iomanip>

namespace catalogue {

std::ostream& operator<<(std::ostream& os, const BusStatistics& bus_info) {
    os << "Bus " << bus_info.number << ": " << bus_info.stops_count << " stops on route, "
       << bus_info.unique_stops_count << " unique stops, ";
    os << bus_info.rout_length << " route length, ";
    os << std::setprecision(6) << bus_info.curvature << " curvature";
    return os;
}

size_t Bus::GetStopsCount() const {
    return (type == RouteType::CIRCLE) ? stop_names.size() : 2 * stop_names.size() - 1;
}

}  // namespace catalog
