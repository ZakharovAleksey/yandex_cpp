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

    /* METHODS FOR MAP IMAGE RENDERING */

    [[nodiscard]] const geo::Coordinates& GetMinStopCoordinates() const;
    [[nodiscard]] const geo::Coordinates& GetMaxStopCoordinates() const;

    [[nodiscard]] const std::set<std::string_view>& GetOrderedBusList() const;

    [[nodiscard]] inline auto GetFinalStops(std::string_view bus_name) const {
        auto bus = buses_.at(bus_name);

        std::vector<std::shared_ptr<Stop>> stops;

        if (bus->stop_names.empty())
            return std::make_pair(bus, stops);

        if (bus->type == RouteType::CIRCLE) {
            // In a circular route, the first stop on the route is considered the final stop
            stops.emplace_back(stops_.at(bus->stop_names.front()));
        } else if (bus->type == RouteType::TWO_DIRECTIONAL) {
            // In a non-circular route, the first and the last stops on the route are considered the final stops
            stops.emplace_back(stops_.at(bus->stop_names.front()));

            // TODO: check here if we could compare by names only
            if (bus->stop_names.front() != bus->stop_names.back())
                stops.emplace_back(stops_.at(bus->stop_names.back()));
        }

        return std::make_pair(bus, stops);
    }

    [[nodiscard]] inline auto GetRouteInfo(std::string_view bus_name, bool include_backward_way = true) const {
        auto bus = buses_.at(bus_name);

        std::vector<std::shared_ptr<Stop>> stops;
        stops.reserve(bus->GetStopsCount());

        // Forward way
        for (std::string_view stop : bus->stop_names)
            stops.emplace_back(stops_.at(stop));

        // Backward way
        if (include_backward_way && bus->type == catalogue::RouteType::TWO_DIRECTIONAL) {
            for (auto stop = std::next(bus->stop_names.rbegin()); stop != bus->stop_names.rend(); ++stop)
                stops.emplace_back(stops_.at(*stop));
        }

        return std::make_pair(std::move(bus), std::move(stops));
    }

private:  // Types
    struct StopPointersPairHash {
        size_t operator()(const StopPointersPair& pair) const {
            return pair.first->Hash() + prime_number * pair.second->Hash();
        }

    private:
        static const size_t prime_number{31};
    };

private:  // Methods
    int CalculateRouteLength(const std::shared_ptr<Bus>& bus_info) const;
    double CalculateGeographicLength(const std::shared_ptr<Bus>& bus_info) const;

    void UpdateMinMaxStopCoordinates(const geo::Coordinates& coordinates);

private:  // Fields
    std::deque<Stop> stops_storage_;
    std::unordered_map<std::string_view, std::shared_ptr<Stop>> stops_;

    std::deque<Bus> buses_storage_;
    std::unordered_map<std::string_view, std::shared_ptr<Bus>> buses_;

    std::unordered_map<std::string_view, std::set<std::string_view>> buses_through_stop_;
    std::unordered_map<StopPointersPair, int, StopPointersPairHash> distances_between_stops_;

    // Fields required for map image rendering
    // TODO: move std::numeric_limits<double>::max() -> to constant
    geo::Coordinates coordinates_min_{std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    geo::Coordinates coordinates_max_{std::numeric_limits<double>::min(), std::numeric_limits<double>::min()};

    // We use unordered containers for faster search in queries.
    // Ordered list in necessary for image rendering only
    std::set<std::string_view> ordered_bus_list;
};

}  // namespace catalogue