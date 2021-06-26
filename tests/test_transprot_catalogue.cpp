//
// Created by azakharov on 6/25/2021.
//

#include <gtest/gtest.h>

#include "../src/sprint_9/input_reader.h"

using namespace std::literals;
using namespace input_utils;

TEST(ParseStopInput, CorrectStopNameParsing){
    std::string_view input = "Stop OnlyLetters: 55, 37"sv;
    auto [bus, _] = ParseBusStopInput(input);
    EXPECT_EQ("OnlyLetters", bus.name) << "Pa"
}

TEST(InputParsing, BusStopsParser) {
    const std::string expected_bus_name = "My Stop 1 Name";
    const double threshold{1e-3};
    std::string_view input;
    {
        input = "Stop My Stop 1 Name: 55.611087, 37.20829"sv;

        auto [bus, distances_to_stops] = ParseBusStopInput(input);
        EXPECT_EQ(bus.name, expected_bus_name);
        EXPECT_NEAR(bus.point.lat, 55.611087, threshold);
        EXPECT_NEAR(bus.point.lng, 37.20829, threshold);
        EXPECT_TRUE(distances_to_stops.empty());
    }
    {
        input = "Stop My Stop 1 Name: -55.611087, 37.20829"sv;

        auto [bus, distances_to_stops] = ParseBusStopInput(input);
        EXPECT_EQ(bus.name, expected_bus_name);
        EXPECT_NEAR(bus.point.lat, -55.611087, threshold);
        EXPECT_NEAR(bus.point.lng, 37.20829, threshold);
        EXPECT_TRUE(distances_to_stops.empty());
    }
    {
        input = "Stop My Stop 1 Name: 55.611087, -37.20829"sv;

        auto [bus, distances_to_stops] = ParseBusStopInput(input);
        EXPECT_EQ(bus.name, expected_bus_name);
        EXPECT_NEAR(bus.point.lat, 55.611087, threshold);
        EXPECT_NEAR(bus.point.lng, -37.20829, threshold);
        EXPECT_TRUE(distances_to_stops.empty());
    }
}