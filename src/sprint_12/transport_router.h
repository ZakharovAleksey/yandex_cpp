#pragma once

#include <optional>
#include <string_view>
#include <vector>

namespace routing {

struct Settings {
    /// Velocity in km per hour
    int bus_velocity_{0};
    /// Wait time in minutes
    int bus_wait_time_{0};
};

struct RouteBase {
    std::string type_;
    double time_;

    std::optional<std::string> stop_name_{std::nullopt};
    std::optional<std::string> bus_{std::nullopt};
    std::optional<int> span_count_{std::nullopt};
};

using RouteItems = std::vector<RouteBase>;

struct RouteResponse {
    double total_time_;
    RouteItems items_;
};

class TransportRouter {
public:  // Constructor
    TransportRouter() = default;
    explicit TransportRouter(Settings settings) : settings_(settings) {}

public:  // Methods
    void SetSettings(Settings settings);

    [[nodiscard]] RouteResponse BuildRoute(std::string_view from, std::string_view to) const;

private:
    [[nodiscard]] RouteItems FindRoute(std::string_view from, std::string_view to) const;
    [[nodiscard]] double MeasureTimeOnTheRoute(const RouteItems& route) const;

private:  // Fields
    Settings settings_;
};

}  // namespace routing