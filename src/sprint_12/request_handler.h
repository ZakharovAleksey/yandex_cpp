#pragma once

/*
 * Description: module for the requests processing.
 * Acts as a Facade that simplifies interaction with the transport directory
 */

#include "map_renderer.h"
#include "transport_router.h"

namespace request {

struct ResponseSettings {
    routing::Settings routing;
    render::Visualization visualization;
};

/// @brief Class acts as a facade for the methods, required during the response creation
class RequestHandler {
public:  // Constructor
    RequestHandler(const catalogue::TransportCatalogue& db, ResponseSettings settings);

public:  // Methods
    std::optional<catalogue::BusStatistics> GetBusStat(const std::string_view& bus_name) const;
    std::unique_ptr<std::set<std::string_view>> GetBusesThroughTheStop(const std::string_view& stop_name) const;
    std::string RenderMap() const;
    routing::ResponseDataOpt BuildRoute(std::string_view from, std::string_view to) const;

private:  // Fields
    const catalogue::TransportCatalogue& db_;
    ResponseSettings settings_;
    // Mutable, because it is "Lazy Evaluation" object, creates only in case there is at least one "Route" request
    mutable routing::TransportRouterOpt router_{std::nullopt};
};

/// @brief Main function to process the requests
/// @param input stream to read requests from
/// @param output stream to put responses to
void ProcessTransportCatalogueQuery(std::istream& input, std::ostream& output);

}  // namespace request