#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>

#include "src/sprint_10/json.h"

using namespace json;
using namespace std::literals;

namespace {

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

std::string Print(const Node& node) {
    std::ostringstream out;
    Print(Document{node}, out);
    return out.str();
}

void MustFailToLoad(const std::string& s) {
    try {
        LoadJSON(s);
        std::cerr << "ParsingError exception is expected on '"sv << s << "'"sv << std::endl;
        assert(false);
    } catch (const json::ParsingError&) {
        // ok
    } catch (const std::exception& e) {
        std::cerr << "exception thrown: "sv << e.what() << std::endl;
        assert(false);
    } catch (...) {
        std::cerr << "Unexpected error"sv << std::endl;
        assert(false);
    }
}

template <typename Fn>
void MustThrowLogicError(Fn fn) {
    try {
        fn();
        std::cerr << "logic_error is expected"sv << std::endl;
        assert(false);
    } catch (const std::logic_error&) {
        // ok
    } catch (const std::exception& e) {
        std::cerr << "exception thrown: "sv << e.what() << std::endl;
        assert(false);
    } catch (...) {
        std::cerr << "Unexpected error"sv << std::endl;
        assert(false);
    }
}

void TestErrorHandling() {
    MustFailToLoad("["s);
    MustFailToLoad("]"s);

    MustFailToLoad("{"s);
    MustFailToLoad("}"s);

    MustFailToLoad("\"hello"s);  // незакрытая кавычка

    MustFailToLoad("tru"s);
    MustFailToLoad("fals"s);
    MustFailToLoad("nul"s);

    Node dbl_node{3.5};
    MustThrowLogicError([&dbl_node] { dbl_node.AsInt(); });
    MustThrowLogicError([&dbl_node] { dbl_node.AsString(); });
    MustThrowLogicError([&dbl_node] { dbl_node.AsArray(); });

    Node array_node{Array{}};
    MustThrowLogicError([&array_node] { array_node.AsMap(); });
    MustThrowLogicError([&array_node] { array_node.AsDouble(); });
    MustThrowLogicError([&array_node] { array_node.AsBool(); });
}

void Benchmark() {
    const auto start = std::chrono::steady_clock::now();
    Array arr;
    arr.reserve(1'000);
    for (int i = 0; i < 1'000; ++i) {
        arr.emplace_back(Dict{
            {"int"s, 42},
            {"double"s, 42.1},
            {"null"s, nullptr},
            {"string"s, "hello"s},
            {"array"s, Array{1, 2, 3}},
            {"bool"s, true},
            {"map"s, Dict{{"key"s, "value"s}}},
        });
    }
    std::stringstream strm;
    json::Print(Document{arr}, strm);
    const auto doc = json::Load(strm);
    assert(doc.GetRoot() == arr);
    const auto duration = std::chrono::steady_clock::now() - start;
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << "ms"sv << std::endl;
}

}  // namespace

int main() {
    TestErrorHandling();
    Benchmark();

    std::cout << "done" << std::endl;
    return 0;
}
