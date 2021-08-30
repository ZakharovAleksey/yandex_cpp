#include <fstream>
#include <iostream>

#include "src/sprint_9/input_reader.h"

using namespace std::literals;
using namespace catalog::input_utils;

int main() {
    std::ifstream is("D:\\Additional\\0_workdir\\yandex_cpp\\data.txt"s);
    ParseTransportCatalogueQueries(is);

    return 0;
}