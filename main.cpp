//#include <cassert>
//#include <filesystem>
//#include <fstream>
//#include <iostream>
//#include <sstream>
//#include <string>
//#include <string_view>
//
// using namespace std;
// using filesystem::path;
//
// void PrintTreeHelper(ostream& dst, const path& p, int depth) {
//    auto directory_content = filesystem::directory_iterator(p);
//
//    std::vector<path> dir_content_names;
//    for (const auto& current_path : directory_content)
//        dir_content_names.emplace_back(current_path.path().filename());
//
//    std::sort(dir_content_names.begin(), dir_content_names.end(),
//              [](const path& left, const path& right) { return right < left; });
//
//    for (const auto current_path : dir_content_names) {
//        auto status = filesystem::status(p / current_path);
//
//        if (status.type() == filesystem::file_type::regular) {
//            dst << std::string(depth, ' ') << current_path.string() << '\n';
//        }
//
//        if (status.type() == filesystem::file_type::directory) {
//            dst << std::string(depth, ' ') << current_path.string() << '\n';
//            PrintTreeHelper(dst, p / current_path, depth + 2);
//        }
//    }
//}
//
//// напишите эту функцию
// void PrintTree(ostream& dst, const path& p) {
//     dst << p.filename().string() << '\n';
//     PrintTreeHelper(dst, p, 2);
// }

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using namespace std;
using filesystem::path;

namespace include_pattern {
static const std::regex relative{"\\s*#\\s*include\\s*\"([^\"]*)\"\\s*"s};
static const std::regex absolute{"\\s*#\\s*include\\s*<([^>]*)>\\s*"s};
}  // namespace include_pattern

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

bool is_file_exists(const path& file_path, std::ifstream& input) {
    if (!exists(file_path))
        return false;

    error_code error;
    auto file_status = status(file_path, error);
    if (error)
        return false;

    if (file_status.type() != filesystem::file_type::regular)
        return false;

    input.open(file_path);
    return input.is_open();
}

void error_message(const path& child_file_name, const path& parent_file_name, int line_id) {
    // clang-format off
    std::cout << "unknown include file "s << child_file_name.generic_string()
              << " at file "s << parent_file_name.generic_string()
              << " at line "s << line_id << std::endl;
    // clang-format on
}

bool PreprocessHelper(std::ifstream& in, std::ofstream& out, const path& parent_path,
                      const std::vector<path>& includes) {
    int line_id{1};

    std::string line;
    std::smatch match;

    while (std::getline(in, line)) {
        bool look_relative_in_absolute{false};

        if (std::regex_match(line, match, include_pattern::relative)) {
            path relative_include = parent_path.parent_path() / path(match[1].str());
            std::ifstream in_child;

            if (is_file_exists(relative_include, in_child)) {
                if (!PreprocessHelper(in_child, out, relative_include, includes))
                    return false;
            } else
                look_relative_in_absolute = true;
        }

        // !!! In we need to look for relative path in absolute, std::regex_match() below won't work, so we use match
        // from the previous if clause !!!
        if (look_relative_in_absolute || std::regex_match(line, match, include_pattern::absolute)) {
            bool is_found{false};
            auto include_name = path(match[1].str());
            std::ifstream in_child;

            for (const auto& include_path : includes) {
                if (path child_file = include_path / include_name; is_file_exists(child_file, in_child)) {
                    is_found = true;
                    if (!PreprocessHelper(in_child, out, child_file, includes))
                        return false;
                    break;
                }
            }

            if (!is_found) {
                error_message(include_name, parent_path, line_id);
                return false;
            }
        }

        if (!std::regex_match(line, match, include_pattern::absolute) &&
            !std::regex_match(line, match, include_pattern::relative)) {
            out << line << '\n';
        }

        ++line_id;
    }

    return true;
}

// напишите эту функцию
bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
    std::ifstream in;
    std::ofstream out(out_file);

    if (!is_file_exists(in_file, in))
        return false;

    const path root_dir = in_file.parent_path();

    return PreprocessHelper(in, out, in_file, include_directories);
}

string GetFileContents(string file) {
    ifstream stream(file);

    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

//#include <deque>
//#include <iostream>
//#include <map>
//#include <memory>
//#include <numeric>
//#include <set>
//
// using namespace std;
//
// struct BookingInfo {
//    int client_id{0};
//
//    int64_t booking_time_{0};
//    int room_count{0};
//};
//
// using BoolingQueue = std::deque<BookingInfo>;
//
// class HotelManager {
// public:
//    void Book(const std::string& hotel_name, BookingInfo info) {
//        if (auto position = storage_.find(hotel_name); position != storage_.end()) {
//            position->second.emplace_back(info);
//        } else {
//            storage_[hotel_name] = {info};
//        }
//
//        remove_outdated_bookings(info.booking_time_);
//    }
//
//    int ComputeClientCount(std::string_view hotel_name) const {
//        std::set<int> clients;
//
//        if (auto position = storage_.find(hotel_name); position != storage_.end()) {
//            for (const auto& info : position->second)
//                clients.insert(info.client_id);
//        }
//        return clients.size();
//    }
//
//    int ComputeRoomCount(std::string_view hotel_name) const {
//        int rooms_count{0};
//
//        auto add_room = [](const int& current_rooms_count, const BookingInfo& info) {
//            return current_rooms_count + info.room_count;
//        };
//
//        if (auto position = storage_.find(hotel_name); position != storage_.end()) {
//            rooms_count = std::accumulate(position->second.begin(), position->second.end(), 0, add_room);
//        }
//
//        return rooms_count;
//    }
//
// private:
//    void remove_outdated_bookings(int64_t last_booking_time) {
//        int64_t min_booking_time = last_booking_time - kSecondsPerDay;
//        for (auto& [_, bookings] : storage_) {
//            int remove_count = get_booking_remove_count(bookings, min_booking_time);
//            bookings.erase(bookings.begin(), bookings.begin() + remove_count);
//        }
//    }
//
//    inline int get_booking_remove_count(BoolingQueue bookings, int64_t min_booking_time) const {
//        int count{0};
//
//        for (const auto& booking : bookings) {
//            if (booking.booking_time_ <= min_booking_time) {
//                ++count;
//            } else {
//                break;
//            }
//        }
//        return count;
//    }
//
// private:
//    static const int64_t kSecondsPerDay{86400};
//
// private:
//    std::map<std::string, BoolingQueue, std::less<>> storage_;
//};
//
// int main() {
//    HotelManager manager;
//
//    std::ifstream in("D:\\education\\cpp\\yandex_cpp\\data\\local\\input.txt");
//
//    int query_count;
//    in >> query_count;
//
//    for (int query_id = 0; query_id < query_count; ++query_id) {
//        string query_type;
//        in >> query_type;
//
//        if (query_type == "BOOK") {
//            int64_t time;
//            in >> time;
//            string hotel_name;
//            in >> hotel_name;
//            int client_id, room_count;
//            in >> client_id >> room_count;
//            manager.Book(hotel_name, {client_id, time, room_count});
//        } else {
//            string hotel_name;
//            in >> hotel_name;
//            if (query_type == "CLIENTS") {
//                cout << manager.ComputeClientCount(hotel_name) << "\n";
//            } else if (query_type == "ROOMS") {
//                cout << manager.ComputeRoomCount(hotel_name) << "\n";
//            }
//        }
//    }
//
//    return 0;
//}

#include <iostream>

#include "src/sprint_11/json_builder.h"

using namespace std;

int main() {
    //    json_11::Print(json_11::Document{json_11::Builder{}
    //                                         .StartDict()
    //                                         .Key("key1"s)
    //                                         .Value(123)
    //                                         .Key("key2"s)
    //                                         .Value("value2"s)
    //                                         .Key("key3"s)
    //                                         .StartArray()
    //                                         .Value(456)
    //                                         .StartDict()
    //                                         .EndDict()
    //                                         .StartDict()
    //                                         .Key(""s)
    //                                         .Value(nullptr)
    //                                         .EndDict()
    //                                         .Value(""s)
    //                                         .EndArray()
    //                                         .EndDict()
    //                                         .Build()},
    //                   cout);
    //    cout << endl;

    // json_11::Print(json_11::Document{json_11::Builder{}.Value("just a string"s).Build()}, cout);

    //    json_11::Print(json_11::Document{json_11::Builder{}
    //                                         .StartArray()
    //                                         .Value("just a string"s)
    //                                         .StartArray()
    //                                         .Value(123)
    //                                         .Value(123.4)
    //                                         .StartArray()
    //                                         .Value("lol"s)
    //                                         .StartArray()
    //                                         .Value("kek"s)
    //                                         .Value("azaza"s)
    //                                         .EndArray()
    //                                         .EndArray()
    //                                         .EndArray()
    //                                         .EndArray()
    //                                         .Build()},
    //                   cout);

    //    json_11::Print(json_11::Document{json_11::Builder{}
    //                                         .StartDict()
    //                                         .Key("just a string"s)
    //                                         .Value(123)
    //                                         .Key("dict"s)
    //                                         .StartDict()
    //                                         .Key("int"s)
    //                                         .Value(123)
    //                                         .Key("double"s)
    //                                         .Value(123.4)
    //                                         .EndDict()
    //                                         .EndDict()
    //                                         .Build()},
    //                   cout);

    //    json_11::Print(json_11::Document{json_11::Builder{}
    //                                         .StartArray()
    //                                         .Value(123)
    //                                         .StartDict()
    //                                         .Key("dict"s)
    //                                         .StartDict()
    //                                         .Key("int"s)
    //                                         .Value(123)
    //                                         .Key("double"s)
    //                                         .Value(123.4)
    //                                         .EndDict()
    //                                         .EndDict()
    //                                         .EndArray()
    //                                         .Build()},
    //                   cout);

    cout << endl;

    try {
        json_11::Builder{}.Value("test"s).StartDict().EndDict().Build();

    } catch (std::logic_error& e) {
        std::cout << "exception: " << e.what() << std::endl;
    }
}