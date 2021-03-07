#include <iostream>
#include <fstream>

#include "sprint_2/utils.h"


using namespace std;

int main() {
    using namespace utils::transport;
    int query_count;
    Query q;

    std::ifstream inp;
    inp.open("D:/Additional/0_workdir/yandex_cpp/data.txt");

    inp >> query_count;

    BusManager bm;
    for (int i = 0; i < query_count; ++i) {
        inp >> q;
        switch (q.type) {
            case QueryType::NewBus:
                bm.AddBus(q.bus, q.stops);
                break;
            case QueryType::BusesForStop:
                cout << bm.GetBusesForStop(q.stop) << endl;
                break;
            case QueryType::StopsForBus:
                cout << bm.GetStopsForBus(q.bus) << endl;
                break;
            case QueryType::AllBuses:
                cout << bm.GetAllBuses() << endl;
                break;
        }
    }
    inp.close();
}