#include <iostream>

#include "src/sprint_8/search_server.h"

using namespace std;

using namespace sprint_8::server;
using namespace sprint_8::server::utils;

#include <string>
#include <thread>

enum class WorkerStatus { FINISHED, IN_PROGRESS };

static WorkerStatus CurrentStatus = WorkerStatus::IN_PROGRESS;

void DoWorkInThread() {
    using namespace std::literals::chrono_literals;
    using namespace std::literals;

    auto thread_id = std::this_thread::get_id();
    while (CurrentStatus == WorkerStatus::IN_PROGRESS) {
        std::cout << "Doing my work in thread " << thread_id << "..."s << std::endl;
        std::this_thread::sleep_for(1s);
    }
}

#include <fstream>
#include <map>

using namespace std::literals;

using MyPair = std::pair<std::string, int>;

#include <algorithm>
#include <vector>

using namespace std;

#include <array>
#include <iomanip>
#include <list>
#include <sstream>

using namespace std;

class VehiclePlate {
public:
    VehiclePlate(char l0, char l1, int digits, char l2, int region)
        : letters_{l0, l1, l2}, digits_(digits), region_(region) {}

    string ToString() const {
        ostringstream out;
        out << letters_[0] << letters_[1];
        // чтобы дополнить цифровую часть номера слева нулями
        // до трёх цифр, используем подобные манипуляторы:
        // setfill задаёт символ для заполнения,
        // right задаёт выравнивание по правому краю,
        // setw задаёт минимальное желаемое количество знаков
        out << setfill('0') << right << setw(3) << digits_;
        out << letters_[2] << setw(2) << region_;

        return out.str();
    }

    int Hash() const {
        return digits_;
    }

    friend bool operator==(const VehiclePlate& left, const VehiclePlate& right);

private:
    array<char, 3> letters_;
    int digits_;
    int region_;
};

ostream& operator<<(ostream& out, VehiclePlate plate) {
    out << plate.ToString();
    return out;
}

bool operator==(const VehiclePlate& left, const VehiclePlate& right) {
    return std::equal(left.letters_.begin(), left.letters_.end(), right.letters_.begin()) &&
           left.digits_ == right.digits_ && left.region_ == right.region_;
}

template <typename T>
class HashableContainer {
public:
    void Insert(T element) {
        int index = element.Hash();

        if (index >= int(elements_.size()))
            elements_.resize(index * 2 + 1);

        auto& bucket = elements_[index];
        if (auto position = std::find(bucket.begin(), bucket.end(), element); position == bucket.end())
            elements_[index].push_back(std::move(element));
    }

    void PrintAll(ostream& out) const {
        for (const auto& node : elements_) {
            for (const auto& value : node)
                out << value << std::endl;
        }
    }

    const auto& GetVector() const {
        return elements_;
    }

private:
    vector<std::vector<T>> elements_;
};

#include <regex>
#include <string>

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