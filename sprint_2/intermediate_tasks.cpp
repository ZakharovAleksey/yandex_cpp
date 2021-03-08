//
// Created by azakharov on 3/6/2021.
//

#include "intermediate_tasks.h"

#include <cassert>
#include <algorithm>
#include <iterator>

namespace intermediate
{
    bool IsPalindrome(const std::string& text) {
        std::string text_copy {text};
        text_copy.erase(std::remove_if(text_copy.begin(), text_copy.end(), [](char s) {return s == ' ';}), text_copy.end());

        // If string ie empty or from space only - not a palindrome
        if (text_copy.empty())
            return false;

        size_t text_length = text_copy.size();

        for(size_t left_id = 0; left_id < text_copy.size() / 2; ++left_id ){
            size_t right_id = text_length - 1 - left_id;
            if (text_copy[left_id] != text_copy[right_id])
                return false;
        }

        return true;
    }

    void Synonyms::Add(const std::string &first_word, const std::string &second_word){
        synonyms_[first_word].insert(second_word);
        synonyms_[second_word].insert(first_word);
    }

    size_t Synonyms::GetSynonymCount(const std::string &word) const {
        if (synonyms_.count(word) != 0) {
            return synonyms_.at(word).size();
        }
        return 0u;
    }

    bool Synonyms::AreSynonyms(const std::string &first_word, const std::string &second_word) const {
        if (synonyms_.count(first_word) == 0)
            return false;

        return synonyms_.at(first_word).count(second_word) > 0;
    }

    namespace transport
    {
        std::istream &operator>>(std::istream &is, Query &q) {
            using namespace std;
            q = {};

            std::string operation_code;
            is >> operation_code;

            if (operation_code == "NEW_BUS"s) {
                q.type = QueryType::NewBus;
                is >> q.bus;

                int stop_count{0};
                is >> stop_count;
                q.stops.reserve(stop_count);

                std::string cur_stop;
                for (size_t i = 0; i < stop_count; ++i) {
                    is >> cur_stop;
                    q.stops.emplace_back(cur_stop);
                }

            } else if (operation_code == "BUSES_FOR_STOP"s) {
                q.type = QueryType::BusesForStop;
                is >> q.stop;
            } else if (operation_code == "STOPS_FOR_BUS"s) {
                q.type = QueryType::StopsForBus;
                is >> q.bus;
            } else if (operation_code == "ALL_BUSES"s) {
                q.type = QueryType::AllBuses;
            }
            return is;
        }

        std::ostream &operator<<(std::ostream &os, const BusesForStopResponse &response) {
            using namespace std;

            switch (response.status) {
                case ResponseStatus::Bad:
                    os << "No stop"s;
                    break;
                case ResponseStatus::Ok:
                    std::copy(response.stops.begin(), response.stops.end(),
                              std::ostream_iterator<std::string>(os, " "));
                    break;
            }

            return os;
        }

        std::ostream &operator<<(std::ostream &os, const StopsForBusResponse &response) {
            using namespace std;

            switch (response.status) {
                case ResponseStatus::Bad:
                    os << "No bus"s;
                    break;

                case ResponseStatus::Ok: {
                    int line_id = 0;
                    for (const auto&[stop, buses] : response.interchange) {
                        if (line_id++ != 0)
                            os << std::endl;

                        std::cout << "Stop "s << stop << ": "s;
                        if (buses.empty())
                            cout << "no interchange"s;
                        else
                            std::copy(buses.begin(), buses.end(), std::ostream_iterator<std::string>(os, " "));
                    }

                    break;
                }
            }

            return os;
        }

        std::ostream &operator<<(std::ostream &os, const AllBusesResponse &response) {
            using namespace std;

            switch (response.status) {
                case ResponseStatus::Bad:
                    os << "No buses"s;
                    break;
                case ResponseStatus::Ok: {
                    int id = 0;
                    for (const auto&[bus, stops] : response.buses_to_stops) {
                        if (id++ != 0)
                            std::cout << std::endl;
                        os << "Bus "s << bus << ": "s;
                        std::copy(stops.begin(), stops.end(), std::ostream_iterator<std::string>(os, " "));
                    }
                    break;
                }
            }

            return os;
        }

        void BusManager::AddBus(const std::string &bus, const std::vector<std::string> &stops) {
            buses_to_stops_[bus] = stops;
            std::for_each(stops.begin(), stops.end(), [&](const std::string &stop) {
                stops_to_buses_[stop].emplace_back(bus);
            });
        }

        BusesForStopResponse BusManager::GetBusesForStop(const std::string &stop) const {
            if (stops_to_buses_.count(stop) == 0)
                return {};

            return {ResponseStatus::Ok, stops_to_buses_.at(stop)};
        }

        StopsForBusResponse BusManager::GetStopsForBus(const std::string &bus) const {
            if (buses_to_stops_.count(bus) == 0)
                return {};

            InterchangeInfo interchange;
            for (const auto &stop : buses_to_stops_.at(bus)) {
                std::vector<std::string> buses;
                std::copy_if(stops_to_buses_.at(stop).begin(), stops_to_buses_.at(stop).end(),
                             std::back_inserter(buses),
                             [bus](const std::string &other_bus) {
                                 return other_bus != bus;
                             });
                interchange.emplace_back(std::make_pair(stop, buses));
            }
            return {ResponseStatus::Ok, interchange};
        }

        AllBusesResponse BusManager::GetAllBuses() const {
            if (buses_to_stops_.empty())
                return {};
            return {ResponseStatus::Ok, buses_to_stops_};
        }
    }

}