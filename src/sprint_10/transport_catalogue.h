#pragma once

/*
 * Description: transport directory module
 */

#include <deque>
#include <optional>
#include <unordered_map>

#include "domain.h"

namespace catalogue {

class TransportCatalogue {
public:  // Constructors
    TransportCatalogue() = default;

public:  // Methods
    void AddStop(Stop stop);
    void AddBus(Bus bus);
    void AddDistance(std::string_view stop_from, std::string_view stop_to, int distance);

    [[nodiscard]] std::optional<BusStatistics> GetBusStatistics(std::string_view bus_number) const;
    [[nodiscard]] const std::set<std::string_view>* GetBusesPassingThroughTheStop(std::string_view stop_name) const;

    [[nodiscard]] const geo::Coordinates& GetMinStopCoordinates() const;
    [[nodiscard]] const geo::Coordinates& GetMaxStopCoordinates() const;

private:  // Types
    struct StopPointersPairHash {
        size_t operator()(const StopPointersPair& pair) const {
            return pair.first->Hash() + prime_number * pair.second->Hash();
        }

    private:
        static const size_t prime_number{31};
    };

private:  // Methods
    int CalculateRouteLength(const Bus* bus_info) const;
    double CalculateGeographicLength(const Bus* bus_info) const;

    void UpdateMinMaxStopCoordinates(const geo::Coordinates& coordinates);

private:  // Fields
    std::deque<Stop> stops_storage_;
    std::unordered_map<std::string_view, const Stop*> stops_;

    std::deque<Bus> buses_storage_;
    std::unordered_map<std::string_view, const Bus*> buses_;

    std::unordered_map<std::string_view, std::set<std::string_view>> buses_through_stop_;
    std::unordered_map<StopPointersPair, int, StopPointersPairHash> distances_between_stops_;

    // Fields required for map image rendering
    // TODO: move std::numeric_limits<double>::max() -> to constant
    geo::Coordinates coordinates_min_{std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    geo::Coordinates coordinates_max_{std::numeric_limits<double>::min(), std::numeric_limits<double>::min()};
};

}  // namespace catalogue