#include <chrono>
#include <fstream>
#include <iostream>

#include "src/json_reader.h"
#include "src/request_handler.h"

using namespace catalogue;

int main() {
    //    TODO: почитать аро устройство проекта - как расположить
    std::ifstream in_make_base{"D:\\education\\cpp\\yandex_cpp\\data\\local\\0_tc_final_input_make_base.json"};
    std::ifstream in_process_request{
        "D:\\education\\cpp\\yandex_cpp\\data\\local\\0_tc_final_input_process_requests.json"};

    std::ofstream out{"D:\\education\\cpp\\yandex_cpp\\data\\local\\0_tc_final_output_route.json"};

    try {
        auto start = std::chrono::system_clock::now();

        request::ProcessTransportCatalogueQuery(in_make_base, out);
        request::ProcessTransportCatalogueQuery(in_process_request, out);

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << elapsed.count() << "ms." << '\n';
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    std::cout << "done" << std::endl;
    return 0;
}
