#pragma once

namespace routing {

struct Settings {
    /// Velocity in km per hour
    int bus_velocity_{0};
    /// Wait time in minutes
    int bus_wait_time_{0};
};

class TransportRouter {
public:  // Constructor
    explicit TransportRouter(Settings settings) : settings_(settings) {}

private:  // Fields
    Settings settings_;
};

}  // namespace routing