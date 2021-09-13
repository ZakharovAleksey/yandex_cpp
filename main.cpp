#define _USE_MATH_DEFINES
#include <cassert>
#include <cmath>
#include <fstream>
#include <string_view>

// #include "src/sprint_10/svg.h"

using namespace std::literals;
// using namespace svg;

/*
Пример использования библиотеки. Он будет компилироваться и работать, когда вы реализуете
все классы библиотеки.
*/

// Выполняет линейную интерполяцию значения от from до to в зависимости от параметра t
// uint8_t Lerp(uint8_t from, uint8_t to, double t) {
//    return static_cast<uint8_t>(std::round((to - from) * t + from));
//}
//
//// Выполняет линейную интерполяцию Rgb цвета от from до to в зависимости от параметра t
// svg::Rgb Lerp(svg::Rgb from, svg::Rgb to, double t) {
//     return {Lerp(from.red, to.red, t), Lerp(from.green, to.green, t), Lerp(from.blue, to.blue, t)};
// }

// int main() {
//     using namespace svg;
//     using namespace std;
//     Color lol;
//     svg::Color color1;                               // none
//     svg::Color color2 = svg::Rgb{215, 30, 25};       // rgb(215,30,25)
//     svg::Color color3 = svg::NoneColor;              // none
//     svg::Color color4 = svg::Rgba{15, 15, 25, 0.7};  // rgba(15,15,25,0.7)
//     svg::Color color5 = "red"s;                      // red
//     svg::Color color6 = svg::Rgb{};                  // rgb(0,0,0)
//     svg::Color color7 = svg::Rgba{};                 // rgba(0,0,0,1.0);
//
//     for (auto lol : {color1, color2, color3, color4, color5, color6, color7})
//         std::cout << lol << std::endl;
//
//     svg::Rgb rgb{255, 0, 100};
//     assert(rgb.red == 255);
//     assert(rgb.green == 0);
//     assert(rgb.blue == 100);
//
//     svg::Rgb c1;
//     // По умолчанию цвет чёрный: {0, 0, 0}
//     assert(c1.red == 0 && c1.green == 0 && c1.blue == 0);
//     svg::Rgba rgba{100, 20, 50, 0.3};
//     assert(rgba.red == 100);
//     assert(rgba.green == 20);
//     assert(rgba.blue == 50);
//     assert(rgba.opacity == 0.3);
//
//     svg::Rgba color;
//     // По умолчанию цвет чёрный: {0, 0, 0, 1.0}
//     assert(color.red == 0 && color.green == 0 && color.blue == 0 && color.opacity == 1.0);
//
//     Rgb start_color{0, 255, 30};
//     Rgb end_color{20, 20, 150};
//
//     const int num_circles = 10;
//     Document doc;
//     for (int i = 0; i < num_circles; ++i) {
//         const double t = double(i) / (num_circles - 1);
//
//         const Rgb fill_color = Lerp(start_color, end_color, t);
//
//         doc.Add(
//             Circle().SetFillColor(fill_color).SetStrokeColor("black"s).SetCenter({i * 20.0 +
//             40, 40.0}).SetRadius(15));
//     }
//
//     doc.Add(Circle().SetFillColor(Rgba{125, 125, 125, 0.5}));
//
//     // Выводим полученный документ в stdout
//     std::ofstream os;
//     os.open("D:\\Additional\\0_workdir\\yandex_cpp\\lol.svg");
//     doc.Render(os);
//     os.close();
// }

//#include <cassert>
//#include <chrono>
//#include <sstream>
//#include <string_view>
//
//#include "src/sprint_10/json.h"
//
// using namespace json;
// using namespace std::literals;
//
// namespace {
//
// json::Document LoadJSON(const std::string& s) {
//    std::istringstream strm(s);
//    return json::Load(strm);
//}
//
//// Раскомментируйте эти функции по мере того, как реализуете недостающий функционал
//
// std::string Print(const Node& node) {
//    std::ostringstream out;
//    Print(Document{node}, out);
//    return out.str();
//}
//
// void MustFailToLoad(const std::string& s) {
//    try {
//        LoadJSON(s);
//        std::cerr << "ParsingError exception is expected on '"sv << s << "'"sv << std::endl;
//        assert(false);
//    } catch (const json::ParsingError&) {
//        // ok
//    } catch (const std::exception& e) {
//        std::cerr << "exception thrown: "sv << e.what() << std::endl;
//        assert(false);
//    } catch (...) {
//        std::cerr << "Unexpected error"sv << std::endl;
//        assert(false);
//    }
//}
//
// template <typename Fn>
// void MustThrowLogicError(Fn fn) {
//    try {
//        fn();
//        std::cerr << "logic_error is expected"sv << std::endl;
//        assert(false);
//    } catch (const std::logic_error&) {
//        // ok
//    } catch (const std::exception& e) {
//        std::cerr << "exception thrown: "sv << e.what() << std::endl;
//        assert(false);
//    } catch (...) {
//        std::cerr << "Unexpected error"sv << std::endl;
//        assert(false);
//    }
//}
//
// void TestNull() {
//    Node null_node;
//    assert(null_node.IsNull());
//
//    Node null_node1{nullptr};
//    assert(null_node1.IsNull());
//
//    assert(Print(null_node) == "null"s);
//
//    const Node node = LoadJSON("null"s).GetRoot();
//    assert(node.IsNull());
//    assert(node == null_node);
//}
//
// void TestNumbers() {
//    Node int_node{42};
//    assert(int_node.IsInt());
//    assert(int_node.AsInt() == 42);
//    // целые числа являются подмножеством чисел с плавающей запятой
//    assert(int_node.IsDouble());
//    // Когда узел хранит int, можно получить соответствующее ему double-значение
//    assert(int_node.AsDouble() == 42.0);
//    assert(!int_node.IsPureDouble());
//
//    Node dbl_node{123.45};
//    assert(dbl_node.IsDouble());
//    assert(dbl_node.AsDouble() == 123.45);
//    assert(dbl_node.IsPureDouble());  // Значение содержит число с плавающей запятой
//    assert(!dbl_node.IsInt());
//
//    assert(Print(int_node) == "42"s);
//    assert(Print(dbl_node) == "123.45"s);
//
//    assert(LoadJSON("42"s).GetRoot() == int_node);
//    assert(LoadJSON("123.45"s).GetRoot() == dbl_node);
//    assert(LoadJSON("0.25"s).GetRoot().AsDouble() == 0.25);
//    assert(LoadJSON("3e5"s).GetRoot().AsDouble() == 3e5);
//    assert(LoadJSON("1.2e-5"s).GetRoot().AsDouble() == 1.2e-5);
//    assert(LoadJSON("1.2e+5"s).GetRoot().AsDouble() == 1.2e5);
//    assert(LoadJSON("-123456"s).GetRoot().AsInt() == -123456);
//}
//
// void TestStrings() {
//    Node str_node{"Hello, \"everybody\""s};
//    assert(str_node.IsString());
//    assert(str_node.AsString() == "Hello, \"everybody\""s);
//
//    assert(!str_node.IsInt());
//    assert(!str_node.IsDouble());
//
//    assert(Print(str_node) == "\"Hello, \\\"everybody\\\"\""s);
//
//    Node lol{" \r\n \" \t\t \\ "s};
//    assert(Print(lol) == "\" \\r\\n \\\" \\t\\t \\\\ \""s);
//
//    auto lolOut = LoadJSON(Print(lol)).GetRoot();
//    assert(lolOut == lol);
//}
//
// void TestBool() {
//    Node true_node{true};
//    assert(true_node.IsBool());
//    assert(true_node.AsBool());
//
//    Node false_node{false};
//    assert(false_node.IsBool());
//    assert(!false_node.AsBool());
//
//    assert(Print(true_node) == "true"s);
//    assert(Print(false_node) == "false"s);
//
//    assert(LoadJSON("true"s).GetRoot() == true_node);
//    assert(LoadJSON("false"s).GetRoot() == false_node);
//}
//
// void TestArray() {
//    Node arr_node{Array{1, 1.23, "Hello"s}};
//    assert(arr_node.IsArray());
//    const Array& arr = arr_node.AsArray();
//    assert(arr.size() == 3);
//    assert(arr.at(0).AsInt() == 1);
//
//    assert(LoadJSON("[1, 1.23, \"Hello\"]"s).GetRoot() == arr_node);
//    assert(LoadJSON(Print(arr_node)).GetRoot() == arr_node);
//}
//
// void TestMap() {
//    Node dict_node{Dict{{"key1"s, "value1"s}, {"key2"s, 42}}};
//    assert(dict_node.IsMap());
//    const Dict& dict = dict_node.AsMap();
//    assert(dict.size() == 2);
//    assert(dict.at("key1"s).AsString() == "value1"s);
//    assert(dict.at("key2"s).AsInt() == 42);
//
//    assert(LoadJSON("{ \"key1\": \"value1\", \"key2\": 42 }"s).GetRoot() == dict_node);
//    assert(LoadJSON(Print(dict_node)).GetRoot() == dict_node);
//}
//
// void TestErrorHandling() {
//    MustFailToLoad("["s);
//    MustFailToLoad("]"s);
//
//    MustFailToLoad("{"s);
//    MustFailToLoad("}"s);
//
//    MustFailToLoad("\"hello"s);  // незакрытая кавычка
//
//    MustFailToLoad("tru"s);
//    MustFailToLoad("fals"s);
//    MustFailToLoad("nul"s);
//
//    Node dbl_node{3.5};
//    MustThrowLogicError([&dbl_node] { dbl_node.AsInt(); });
//    MustThrowLogicError([&dbl_node] { dbl_node.AsString(); });
//    MustThrowLogicError([&dbl_node] { dbl_node.AsArray(); });
//
//    Node array_node{Array{}};
//    MustThrowLogicError([&array_node] { array_node.AsMap(); });
//    MustThrowLogicError([&array_node] { array_node.AsDouble(); });
//    MustThrowLogicError([&array_node] { array_node.AsBool(); });
//}
//
// void Benchmark() {
//    const auto start = std::chrono::steady_clock::now();
//    Array arr;
//    size_t size{1};
//    arr.reserve(size);
//    for (int i = 0; i < size; ++i) {
//        arr.emplace_back(Dict{
//            {"int"s, 42},
//            {"double"s, 42.1},
//            {"null"s, nullptr},
//            {"string"s, "hello"s},
//            {"array"s, Array{1, 2, 3}},
//            {"bool"s, true},
//            {"map"s, Dict{{"key"s, "value"s}}},
//        });
//    }
//    std::stringstream strm;
//    json::Print(Document{arr}, strm);
//    const auto doc = json::Load(strm);
//    assert(doc.GetRoot() == arr);
//    const auto duration = std::chrono::steady_clock::now() - start;
//    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << "ms"sv << std::endl;
//}
//
//}  // namespace

#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

int main() {
    std::vector<int> a{1, 2, 3, 4};
    std::transform(a.begin(), a.end(), a.begin(), [](int value) { return value * value; });

    auto sum = std::accumulate(a.begin(), a.end(), 0, [](int left, int right) { return left + right; });

    std::copy(a.begin(), a.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "SUM = " << sum << std::endl;


    //    TestNull();
    //    TestNumbers();
    //    TestStrings();
    //    TestBool();
    //    TestArray();
    //    TestMap();

    //    TestErrorHandling();
    // Benchmark();

    std::cout << "done" << std::endl;
}
