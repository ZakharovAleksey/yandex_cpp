//
// Created by azakharov on 3/6/2021.
//

#ifndef YP_CPP1_INTERMEDIATE_TASKS_H
#define YP_CPP1_INTERMEDIATE_TASKS_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <iostream>

namespace intermediate {
    bool IsPalindrome(const std::string &text);

    class Synonyms {
    public:
        void Add(const std::string &first_word, const std::string &second_word);

        size_t GetSynonymCount(const std::string &word) const;

        bool AreSynonyms(const std::string &first_word, const std::string &second_word) const;

    private:
        using WordsDictionary = std::map<std::string, std::set<std::string>>;

    private:
        WordsDictionary synonyms_;
    };

    namespace transport {

        using InterchangeInfo = std::vector<std::pair<std::string, std::vector<std::string>>>;

        enum class QueryType {
            NewBus,
            BusesForStop,
            StopsForBus,
            AllBuses,
        };

        enum class ResponseStatus {
            Bad,
            Ok
        };

        struct Query {
            QueryType type;
            std::string bus;
            std::string stop;
            std::vector<std::string> stops;

            friend std::istream &operator>>(std::istream &is, Query &q);
        };

        struct BusesForStopResponse {
            ResponseStatus status{ResponseStatus::Bad};
            std::vector<std::string> stops;

            friend std::ostream &operator<<(std::ostream &os, const BusesForStopResponse &response);
        };

        struct StopsForBusResponse {
            ResponseStatus status{ResponseStatus::Bad};
            InterchangeInfo interchange;

            friend std::ostream &operator<<(std::ostream &os, const StopsForBusResponse &response);
        };

        struct AllBusesResponse {
            ResponseStatus status{ResponseStatus::Bad};
            std::map<std::string, std::vector<std::string>> buses_to_stops;

            friend std::ostream &operator<<(std::ostream &os, const AllBusesResponse &response);
        };

        class BusManager {
        public:
            void AddBus(const std::string &bus, const std::vector<std::string> &stops);

            BusesForStopResponse GetBusesForStop(const std::string &stop) const;

            StopsForBusResponse GetStopsForBus(const std::string &bus) const;

            AllBusesResponse GetAllBuses() const;

        private:
            using InfoContainer = std::map<std::string, std::vector<std::string>>;

        private:
            InfoContainer buses_to_stops_;
            InfoContainer stops_to_buses_;
        };
    }
}

#endif //YP_CPP1_INTERMEDIATE_TASKS_H
