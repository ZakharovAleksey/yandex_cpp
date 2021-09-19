#include "json_reader.h"

#include <string>

#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace request {

using namespace catalogue;
using namespace request::utils;
using namespace std::literals;

void PrintBusesPassingThroughStop(std::ostream& os, std::string_view stop_name,
                                  const std::set<std::string_view>* buses) {
    if (!buses) {
        os << "Stop " << stop_name << ": not found" << std::endl;
    } else if (buses->empty()) {
        os << "Stop " << stop_name << ": no buses" << std::endl;
    } else {
        os << "Stop " << stop_name << ": buses ";
        size_t index{0u};
        for (std::string_view bus : *buses) {
            if (index++ != 0)
                os << " ";
            os << bus;
        }
        os << std::endl;
    }
}

void ParseTransportCatalogueQueries(std::istream& input_stream) {
    TransportCatalogue catalogue;

    int queries_count{0};
    input_stream >> queries_count;
    input_stream.get();

    std::vector<std::string> bus_queries;
    bus_queries.reserve(queries_count);
    std::vector<std::pair<std::string, std::string>> stop_distances;
    stop_distances.reserve(queries_count);

    std::string query;
    for (int id = 0; id < queries_count; ++id) {
        std::getline(input_stream, query);
        if (query.substr(0, 4) == "Stop"s) {
            auto [stop, is_store_query] = ParseBusStopInput(query);
            if (is_store_query)
                stop_distances.emplace_back(stop.name, std::move(query));
            catalogue.AddStop(std::move(stop));
        } else if (query.substr(0, 3) == "Bus"s) {
            bus_queries.emplace_back(std::move(query));
        }
    }

    for (const auto& [stop_from, query] : stop_distances) {
        for (auto [stop_to, distance] : ParsePredefinedDistancesBetweenStops(query))
            catalogue.AddDistance(stop_from, stop_to, distance);
    }

    for (const auto& bus_query : bus_queries)
        catalogue.AddBus(ParseBusRouteInput(bus_query));

    input_stream >> queries_count;
    input_stream.get();
    for (int id = 0; id < queries_count; ++id) {
        std::getline(input_stream, query);
        if (query.substr(0, 3) == "Bus"s) {
            std::string_view bus_number = ParseBusStatisticsRequest(query);

            if (auto bus_statistics = catalogue.GetBusStatistics(bus_number)) {
                std::cout << *bus_statistics << std::endl;
            } else {
                std::cout << "Bus " << bus_number << ": not found" << std::endl;
            }
        } else if (query.substr(0, 4) == "Stop"s) {
            std::string_view stop_name = ParseBusesPassingThroughStopRequest(query);
            auto* buses = catalogue.GetBusesPassingThroughTheStop(stop_name);

            PrintBusesPassingThroughStop(std::cout, stop_name, buses);
        }
    }
}

}  // namespace request