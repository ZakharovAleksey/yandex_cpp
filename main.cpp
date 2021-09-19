#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

#include "src/sprint_10/json_reader.h"
#include "src/sprint_10/request_handler.h"

using namespace catalogue;

int main() {
    //    TODO: почитать аро устройство проекта - как расположить
    std::fstream in{"D:\\education\\cpp\\yandex_cpp\\input.json"};
    std::ofstream out{"D:\\education\\cpp\\yandex_cpp\\output.json"};

    try {
        request::ProcessTransportCatalogueQuery(in, out);
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    std::cout << "done" << std::endl;
    return 0;
}