#include <chrono>
#include <fstream>
#include <iostream>

#include "src/sprint_12//json_reader.h"
#include "src/sprint_12/request_handler.h"

#include "src/sprint_13/vector.h"

using namespace catalogue;

int main() {
    //    TODO: почитать аро устройство проекта - как расположить
    std::ifstream in{"D:\\education\\cpp\\yandex_cpp\\data\\local\\input_route.json"};
    std::ofstream out{"D:\\education\\cpp\\yandex_cpp\\data\\local\\output_route.json"};

    try {
        auto start = std::chrono::system_clock::now();

        request::ProcessTransportCatalogueQuery(in, out);

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << elapsed.count() << "ms." << '\n';
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    std::cout << "done" << std::endl;
    return 0;
}
