#include <iostream>
#include <fstream>

#include "src/sprint_9/input_reader.h"
#include "src/sprint_9/stat_reader.h"
#include "src/sprint_9/transport_catalogue.h"

using namespace std::literals;
using namespace input_utils;
using namespace output_utils;
using namespace catalog;

int main() {
    TransportCatalogue catalogue;
    std::ifstream is("D:\\Additional\\0_workdir\\yandex_cpp\\data.txt"s);

    int queries_count{0};
    is >> queries_count;
    is.get();

    std::vector<std::string> bus_queries;
    bus_queries.reserve(queries_count);
    std::vector<std::pair<std::string, std::string>> stop_distances;
    stop_distances.reserve(queries_count);

    std::string query;
    for (int id = 0; id < queries_count; ++id) {
        std::getline(is, query);
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

    is >> queries_count;
    is.get();
    for (int id = 0; id < queries_count; ++id) {
        std::getline(is, query);
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
    return 0;
}