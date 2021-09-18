//
// Created by azakharov on 9/18/2021.
//

#include <gtest/gtest.h>

#include "../src/sprint_10/json.h"

using namespace std::string_literals;
using namespace json;

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

std::string Print(const Node& node) {
    std::ostringstream out;
    Print(Document{node}, out);
    return out.str();
}

TEST(JsonParsing, ParsingEmptyString) {
    EXPECT_THROW([[maybe_unused]] auto value = LoadJSON(""s).GetRoot(), json::ParsingError);
}

TEST(JsonParsing, NullNode) {
    Node node;
    EXPECT_TRUE(node.IsNull()) << "Default constructor should create null node";
    EXPECT_EQ(Print(node), "null"s) << "Print() method for null node does not work properly";

    Node node1{nullptr};
    EXPECT_TRUE(node1.IsNull()) << "Constructor with {nullptr} should create null node";

    const Node node2 = LoadJSON("null"s).GetRoot();
    EXPECT_TRUE(node2.IsNull()) << "Expected null node  with null constructor";

    EXPECT_EQ(node, node2) << "operator== for null node does not work properly";
}

TEST(JsonParsing, NullNodeThrowsException) {
    const std::vector<std::string> inputs{"n"s, "nu"s, "nul"s, "nul1"s, "nullqwe"s, "nul\\t"s};

    for (const auto& input : inputs)
        EXPECT_THROW(LoadJSON(input), json::ParsingError) << "Should throw for incorrect input for " << input;
}

TEST(JsonParsing, BooleanNode) {
    Node true_node{true};
    EXPECT_TRUE(true_node.IsBool());
    EXPECT_TRUE(true_node.AsBool());

    Node false_node{false};
    EXPECT_TRUE(false_node.IsBool());
    EXPECT_FALSE(false_node.AsBool());

    EXPECT_EQ(Print(true_node), "true"s);
    EXPECT_EQ(Print(false_node), "false"s);

    EXPECT_EQ(LoadJSON("true"s).GetRoot(), true_node);
    EXPECT_EQ(LoadJSON("false"s).GetRoot(), false_node);
}

TEST(JsonParsing, BooleanNodeThrowsException) {
    const std::vector<std::string> inputs{"t"s, "tr"s,  "tru"s,  "trus"s,  "trust"s,
                                          "f"s, "fal"s, "fals"s, "falsa"s, "falsqwe"s};

    for (const auto& input : inputs)
        EXPECT_THROW(LoadJSON(input), json::ParsingError) << "Should throw for incorrect input for " << input;
}

TEST(JsonParsing, IntNode) {
    Node int_node{42};

    EXPECT_TRUE(int_node.IsInt());
    EXPECT_EQ(int_node.AsInt(), 42);

    // целые числа являются подмножеством чисел с плавающей запятой
    EXPECT_TRUE(int_node.IsDouble());

    // Когда узел хранит int, можно получить соответствующее ему double-значение
    EXPECT_EQ(int_node.AsDouble(), 42.0);
    EXPECT_FALSE(int_node.IsPureDouble());

    EXPECT_EQ(Print(int_node), "42"s);

    EXPECT_EQ(LoadJSON("42"s).GetRoot(), int_node);
}

TEST(JsonParsing, IntNodeThrowsException) {
    //    TODO: тут продумать все возможные варианты (начинать с 0, ...)
}

TEST(JsonParsing, DoubleNode) {
    Node dbl_node{123.45};

    EXPECT_TRUE(dbl_node.IsDouble());
    EXPECT_EQ(dbl_node.AsDouble(), 123.45);
    EXPECT_TRUE(dbl_node.IsPureDouble());  // Значение содержит число с плавающей запятой
    EXPECT_FALSE(dbl_node.IsInt());

    EXPECT_EQ(Print(dbl_node), "123.45"s);

    EXPECT_EQ(LoadJSON("123.45"s).GetRoot(), dbl_node);
    EXPECT_EQ(LoadJSON("0.25"s).GetRoot().AsDouble(), 0.25);
    EXPECT_EQ(LoadJSON("3e5"s).GetRoot().AsDouble(), 3e5);
    EXPECT_EQ(LoadJSON("1.2e-5"s).GetRoot().AsDouble(), 1.2e-5);
    EXPECT_EQ(LoadJSON("1.2e+5"s).GetRoot().AsDouble(), 1.2e5);
    EXPECT_EQ(LoadJSON("-123456"s).GetRoot().AsInt(), -123456);
}

TEST(JsonParsing, DoubleNodeThrowsException) {
    //    TODO: тут продумать все возможные варианты (начинать с 0, ...)
}

TEST(JsonParsing, StringNode) {
    Node str_node{" \r\n \" \t\t \\ "s};

    EXPECT_TRUE(str_node.IsString());
    EXPECT_EQ(str_node.AsString(), " \r\n \" \t\t \\ "s);
    EXPECT_EQ(Print(str_node), "\" \\r\\n \\\" \\t\\t \\\\ \""s);

    EXPECT_EQ(LoadJSON("\" \\r\\n \\\" \\t\\t \\\\ \""s).GetRoot(), str_node);

    str_node = Node{"Hello, \"everybody\""s};
    EXPECT_TRUE(str_node.IsString());
    EXPECT_EQ(str_node.AsString(), "Hello, \"everybody\""s);
    EXPECT_EQ(Print(str_node), "\"Hello, \\\"everybody\\\"\""s);

    EXPECT_EQ(LoadJSON("\"Hello, \\\"everybody\\\"\"").GetRoot(), str_node);
}

TEST(JsonParsing, StringNodeThrowsException) {
    //    TODO: тут продумать все возможные варианты (начинать с 0, ...)
}

TEST(JsonParsing, EmptyDictNodeParsing) {
    auto node = LoadJSON("{}"s).GetRoot();

    EXPECT_TRUE(node.IsMap());
    EXPECT_TRUE(node.AsMap().empty());
}

TEST(JsonParsing, DictNode) {
    Node dict_node{Dict{{"k0"s, nullptr},
                        {"k1"s, true},
                        {"k2"s, false},
                        {"k3"s, 1},
                        {"k4"s, 1.1},
                        {"k5"s, "Hello"s},
                        {"k6"s, Array{1, 1.1}},
                        {"k7"s, Dict{{"k0"s, Array{}}}}}};

    EXPECT_TRUE(dict_node.IsMap());
    const Dict& dict = dict_node.AsMap();

    EXPECT_EQ(dict.size(), 8);
    EXPECT_EQ(dict.at("k0"s), Node{nullptr});
    EXPECT_EQ(dict.at("k1"s).AsBool(), true);
    EXPECT_EQ(dict.at("k2"s).AsBool(), false);
    EXPECT_EQ(dict.at("k3"s).AsInt(), 1);
    EXPECT_EQ(dict.at("k4"s).AsDouble(), 1.1);
    EXPECT_EQ(dict.at("k5"s).AsString(), "Hello");

    EXPECT_TRUE(dict.at("k6"s).IsArray());
    EXPECT_EQ(dict.at("k6"s).AsArray().size(), 2);
    auto lol = dict.at("k6"s).AsArray();
    EXPECT_EQ(dict.at("k6"s).AsArray().at(0).AsInt(), 1);
    EXPECT_EQ(dict.at("k6"s).AsArray().at(1).AsDouble(), 1.1);

    EXPECT_TRUE(dict.at("k7"s).IsMap());
    EXPECT_EQ(dict.at("k7"s).AsMap().size(), 1);
    EXPECT_TRUE(dict.at("k7"s).AsMap().at("k0"s).IsArray());
    EXPECT_TRUE(dict.at("k7"s).AsMap().at("k0"s).AsArray().empty());

    const std::string expected_print =
        R"({"k0":null, "k1":true, "k2":false, "k3":1, "k4":1.1, "k5":"Hello", "k6":[1, 1.1], "k7":{"k0":[]}})"s;
    auto actual_print = Print(dict_node);

    EXPECT_EQ(actual_print, expected_print);
    EXPECT_EQ(LoadJSON(actual_print).GetRoot(), dict_node);
}

TEST(JsonParsing, DictNodeThrowsException) {

}

TEST(JsonParsing, ArrayNodeWithAndWithoutSpaces) {
    Node array_node{Array{1, 1.1}};

    EXPECT_EQ(LoadJSON("[1,1.1]"s).GetRoot(), array_node) << "Should parse array without spaces between items";
    EXPECT_EQ(LoadJSON("[1, 1.1]"s).GetRoot(), array_node) << "Should parse array without spaces between items";
    EXPECT_EQ(LoadJSON("[1,   1.1]"s).GetRoot(), array_node) << "Should parse array without spaces between items";
}

TEST(JsonParsing, EmptyArrayNodeParsing) {
    auto node = LoadJSON("[]"s).GetRoot();

    EXPECT_TRUE(node.IsArray());
    EXPECT_TRUE(node.AsArray().empty());
}

TEST(JsonParsing, ArrayNode) {
    Node arr_node{
        Array{nullptr, true, false, 1, 1.1, "Hello"s, Array{}, Array{1, 2.1}, Dict{}, Dict{{"key"s, "value"s}}}};
    EXPECT_TRUE(arr_node.IsArray());

    const Array& arr = arr_node.AsArray();
    EXPECT_EQ(arr.size(), 10);

    EXPECT_EQ(arr.at(0), Node{nullptr});
    EXPECT_EQ(arr.at(1).AsBool(), true);
    EXPECT_EQ(arr.at(2).AsBool(), false);
    EXPECT_EQ(arr.at(3).AsInt(), 1);
    EXPECT_EQ(arr.at(4).AsDouble(), 1.1);
    EXPECT_EQ(arr.at(5).AsString(), "Hello"s);
    EXPECT_EQ(arr.at(6).AsArray(), Array{});

    EXPECT_TRUE(arr.at(7).IsArray());
    EXPECT_EQ(arr.at(7).AsArray().at(0), 1);
    EXPECT_EQ(arr.at(7).AsArray().at(1), 2.1);

    EXPECT_EQ(arr.at(8).AsMap(), Dict{});

    EXPECT_TRUE(arr.at(9).IsMap());
    EXPECT_EQ(arr.at(9).AsMap().at("key"s), "value"s);

    const auto expected_print = "[null, true, false, 1, 1.1, \"Hello\", [], [1, 2.1], {}, {\"key\":\"value\"}]"s;
    auto actual_print = Print(arr_node);

    EXPECT_EQ(actual_print, expected_print);
    EXPECT_EQ(LoadJSON(actual_print).GetRoot(), arr_node);
}

TEST(JsonParsing, ArrayNodeThrowsException) {
    EXPECT_THROW(LoadJSON("["s), json::ParsingError);
    EXPECT_THROW(LoadJSON("]"s), json::ParsingError);

    EXPECT_THROW(LoadJSON("[1, 1.1"s), json::ParsingError);
    EXPECT_THROW(LoadJSON("[1, 1.1 1.2]"s), json::ParsingError);
    EXPECT_THROW(LoadJSON("[1  1.1]"s), json::ParsingError);
}
