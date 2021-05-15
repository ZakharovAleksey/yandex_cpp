cmake_minimum_required(VERSION 3.17)
project(yp_cpp1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(BOOST_ROOT "D:/Additional/0_workdir/yandex_cpp/3rdparty/boost_1_75_0")

add_subdirectory(tests)

FIND_PACKAGE(Boost 1.75.0 REQUIRED)
if (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
endif ()

add_executable(yp_cpp1 main.cpp src/sprint_1/server.cpp src/sprint_1/server.h src/intermidiate_tasks/sprint_2_tasks.h src/intermidiate_tasks/sprint_2_tasks.cpp src/sprint_2/testing_framework.h src/sprint_2/server_test.h src/sprint_2/server_test.cpp src/sprint_2/testing_framework.cpp src/intermidiate_tasks/sprint_3_tasks.cpp src/intermidiate_tasks/sprint_3_tasks.h src/intermidiate_tasks/sprint_4_tasks.cpp src/intermidiate_tasks/sprint_4_tasks.h src/sprint_1/print_utils.h src/sprint_4/document.cpp src/sprint_4/document.h src/sprint_4/paginator.h src/sprint_4/request_queue.cpp src/sprint_4/request_queue.h src/sprint_4/search_server.cpp src/sprint_4/search_server.h src/sprint_4/string_processing.cpp src/sprint_4/string_processing.h src/sprint_3/server_search.cpp src/sprint_3/server_search.h src/sprint_1/print_utils.h src/sprint_5/document.cpp src/sprint_5/document.h src/sprint_5/paginator.h src/sprint_5/request_queue.cpp src/sprint_5/request_queue.h src/sprint_5/search_server.cpp src/sprint_5/search_server.h src/sprint_5/string_processing.cpp src/sprint_5/string_processing.h src/sprint_3/server_search.cpp src/sprint_3/server_search.h src/sprint_5/log_duration.h src/sprint_5/log_duration.h src/sprint_6/single_linked_list.h)
if (Boost_FOUND)
    target_link_libraries(yp_cpp1 ${Boost_LIBRARIES})
endif ()