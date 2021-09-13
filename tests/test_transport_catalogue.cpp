//
// Created by azakharov on 6/25/2021.
//

#include <gtest/gtest.h>

#include "../src/sprint_9/geo.h"
#include "../src/sprint_9/input_reader.h"
#include "../src/sprint_9/stat_reader.h"
#include "../src/sprint_9/transport_catalogue.h"

using namespace geo;
using namespace catalog::input_utils;
using namespace catalog::output_utils;

using namespace std::literals;

constexpr double kDoubleThreshold{1e-3};

static const std::map<std::string, std::string> kStopNamesWithErrors = {
    {"StopName"s, "CamelCase letters ONLY"s},
    {"Stop_Name"s, "Snake_Case letters ONLY"s},
    {"Stop Name"s, "letters with spaces"s},
    {"StopName1"s, "letters with numbers"s},
    {"Stop Name 1"s, "letters with spaces and numbers"s},
    {"Stop Name #1"s, "letters with spaces, numbers and special symbols"}};

TEST(InputParsing, BusStopQueryStopName) {
    std::string stop_name;

    for (const auto& [stop_name, error_postfix] : kStopNamesWithErrors) {
        auto [stop, _] = ParseBusStopInput("Stop "s + stop_name + ": 55, 37"s);
        EXPECT_EQ(stop_name, stop.name) << "Incorrect for stop name with " << error_postfix;
    }
}

TEST(InputParsing, BusStopQueryLatitudeLongitude) {
    std::map<std::string, Coordinates> test_input = {
        {"positive int & positive int"s, {55, 35}},   {"positive float & positive int"s, {55.123, 35}},
        {"negative int & positive int"s, {-55, 35}},  {"negative float & positive int"s, {-55.123, 35}},
        {"negative int & negative int"s, {-55, -35}}, {"negative int & negative int"s, {-55.123, -35.321}}};

    auto to_string = [&test_input](const Coordinates& position) {
        return std::to_string(position.lat) + ", "s + std::to_string(position.lng);
    };

    for (const auto& [error_postfix, position] : test_input) {
        auto [stop, _] = ParseBusStopInput("Stop StopName: " + to_string(position));
        EXPECT_NEAR(position.lat, stop.point.lat, kDoubleThreshold)
            << "Incorrect latitude for data format:  " << error_postfix;
        EXPECT_NEAR(position.lng, stop.point.lng, kDoubleThreshold)
            << "Incorrect longitude for data format:  " << error_postfix;
    }
}

TEST(InputParsing, DistancesBetweenStops) {
    auto make_second_stop_name = [](const std::string& name) { return name + " v.2"; };

    auto make_query = [&](const std::string& stop_name) {
        return "Stop Marushkino: 55.595884, 37.209755, 9900m to "s + stop_name + ", 100m to "s +
               make_second_stop_name(stop_name);
    };

    auto [_, has_stops] = ParseBusStopInput("Stop StopName: 55, 37"s);
    EXPECT_FALSE(has_stops) << "Function ParseBusStopInput() handles incorrectly queries without stops";

    std::tie(_, has_stops) = ParseBusStopInput(make_query("StopName"));
    EXPECT_TRUE(has_stops) << "Function ParseBusStopInput() handles incorrectly queries with stops";

    for (const auto& [stop_name, error_postfix] : kStopNamesWithErrors) {
        std::string query = make_query(stop_name);
        const auto actual_data = ParsePredefinedDistancesBetweenStops(query);

        EXPECT_EQ(actual_data.size(), 2) << "Incorrect number of stops has been parsed";

        EXPECT_EQ(actual_data[0].first, stop_name) << "Incorrect stop name parsing";
        EXPECT_EQ(actual_data[0].second, 9900) << "Incorrect distance parsing";

        EXPECT_EQ(actual_data[1].first, make_second_stop_name(stop_name)) << "Incorrect stop name parsing";
        EXPECT_EQ(actual_data[1].second, 100) << "Incorrect distance parsing";
    }
}

TEST(StatParsing, BusNumber) {
    // Stops names are identical to the bus names, that's why we use them
    for (const auto& [bus_name, error_message] : kStopNamesWithErrors) {
        std::string query = "Bus "s + bus_name;
        std::string_view actual_bus_name = ParseBusStatisticsRequest(query);

        EXPECT_EQ(bus_name, actual_bus_name) << "Incorrect bus number for query with " << error_message;
    }
}

TEST(StatParsing, StopName) {
    for (const auto& [stop_name, error_message] : kStopNamesWithErrors) {
        std::string query = "Stop "s + stop_name;
        std::string_view actual_stop_name = ParseBusesPassingThroughStopRequest(query);

        EXPECT_EQ(stop_name, actual_stop_name) << "Incorrect stop for query with " << error_message;
    }
}
