#pragma once

/*
 * Description: transport directory module
 */

#include <deque>
#include <map>
#include <optional>
#include <unordered_map>

#include "domain.h"

namespace catalogue {

using BusStopsStorage = std::pair<std::shared_ptr<Bus>, std::vector<std::shared_ptr<Stop>>>;
using StopsStorage = std::map<std::string_view, std::shared_ptr<Stop>>;

class TransportCatalogue {
public:  // Constructors
    TransportCatalogue() = default;

public:  // Methods
    void AddStop(Stop stop);
    void AddBus(Bus bus);
    void AddDistance(std::string_view stop_from, std::string_view stop_to, int distance);

    [[nodiscard]] std::optional<BusStatistics> GetBusStatistics(std::string_view bus_number) const;
    [[nodiscard]] std::unique_ptr<std::set<std::string_view>> GetBusesPassingThroughTheStop(std::string_view stop_name) const;

    /* METHODS FOR MAP IMAGE RENDERING */

    [[nodiscard]] const geo::Coordinates& GetMinStopCoordinates() const;
    [[nodiscard]] const geo::Coordinates& GetMaxStopCoordinates() const;

    [[nodiscard]] const std::set<std::string_view>& GetOrderedBusList() const;
    [[nodiscard]] BusStopsStorage GetFinalStops(std::string_view bus_name) const;
    [[nodiscard]] BusStopsStorage GetRouteInfo(std::string_view bus_name, bool include_backward_way = true) const;
    [[nodiscard]] StopsStorage GetAllStopsFromRoutes() const;

private:  // Types
    struct StopPointersPairHash {
        size_t operator()(const StopPointersPair& pair) const {
            return pair.first->Hash() + prime_number * pair.second->Hash();
        }

    private:
        static const size_t prime_number{31};
    };

private:  // Methods
    [[nodiscard]] int CalculateRouteLength(const std::shared_ptr<Bus>& bus_info) const;
    [[nodiscard]] double CalculateGeographicLength(const std::shared_ptr<Bus>& bus_info) const;

    void UpdateMinMaxStopCoordinates(const geo::Coordinates& coordinates);

private:  // Fields
    std::deque<Stop> stops_storage_;
    std::unordered_map<std::string_view, std::shared_ptr<Stop>> stops_;

    std::deque<Bus> buses_storage_;
    std::unordered_map<std::string_view, std::shared_ptr<Bus>> buses_;

    std::unordered_map<std::string_view, std::set<std::string_view>> buses_through_stop_;
    std::unordered_map<StopPointersPair, int, StopPointersPairHash> distances_between_stops_;

    // Fields required for map image rendering
    geo::Coordinates coordinates_min_{std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    geo::Coordinates coordinates_max_{std::numeric_limits<double>::min(), std::numeric_limits<double>::min()};

    // We use unordered containers for faster search in queries.
    // Ordered list in necessary for image rendering only
    std::set<std::string_view> ordered_bus_list_;
};

}  // namespace catalogue