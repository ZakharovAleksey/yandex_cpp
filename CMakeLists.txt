cmake_minimum_required(VERSION 3.16)
project(yp_cpp1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

#[[
Compilation flags:
    WSL: -fsanitize=address -g -fno-omit-frame-pointer -fno-optimize-sibling-calls -ltbb -lpthread
    VS: -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer -g -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_LIBCPP_DEBUG=1
]]

if (MSVC)
    # All flags: https://docs.microsoft.com/ru-ru/cpp/build/reference/compiler-options-listed-by-category?view=msvc-160
    add_compile_options(/WX)
else ()
    add_compile_options(-Wall -Wextra)
endif ()

add_subdirectory(tests)

add_executable(yp_cpp1
        main.cpp
        src/intermediate_tasks/sprint_2_tasks.h
        src/intermediate_tasks/sprint_2_tasks.cpp
        src/intermediate_tasks/sprint_3_tasks.cpp
        src/intermediate_tasks/sprint_3_tasks.h
        src/intermediate_tasks/sprint_4_tasks.cpp
        src/intermediate_tasks/sprint_4_tasks.h
        src/intermediate_tasks/sprint_7_tasks.cpp
        src/intermediate_tasks/sprint_7_tasks.h
        src/intermediate_tasks/sprint_8_tasks.h
        src/sprint_1/server.cpp
        src/sprint_1/server.h
        src/sprint_1/print_utils.h
        src/sprint_2/testing_framework.h
        src/sprint_2/server_test.h
        src/sprint_2/server_test.cpp
        src/sprint_2/testing_framework.cpp
        src/sprint_3/server_search.cpp
        src/sprint_3/server_search.h
        src/sprint_3/server_search.cpp
        src/sprint_3/server_search.h
        src/sprint_4/document.cpp
        src/sprint_4/document.h
        src/sprint_4/paginator.h
        src/sprint_4/request_queue.cpp
        src/sprint_4/request_queue.h
        src/sprint_4/search_server.cpp
        src/sprint_4/search_server.h
        src/sprint_4/string_processing.cpp
        src/sprint_4/string_processing.h
        src/sprint_5/document.cpp
        src/sprint_5/document.h
        src/sprint_5/paginator.h
        src/sprint_5/request_queue.cpp
        src/sprint_5/request_queue.h
        src/sprint_5/search_server.cpp
        src/sprint_5/search_server.h
        src/sprint_5/string_processing.cpp
        src/sprint_5/string_processing.h
        src/sprint_5/log_duration.h
        src/sprint_6/single_linked_list.h
        src/sprint_7/array_ptr.h
        src/sprint_7/simple_vector.h
        src/sprint_8/document.cpp
        src/sprint_8/document.h
        src/sprint_8/paginator.h
        src/sprint_8/request_queue.cpp
        src/sprint_8/request_queue.h
        src/sprint_8/search_server.cpp
        src/sprint_8/search_server.h
        src/sprint_8/string_processing.cpp
        src/sprint_8/string_processing.h
        src/sprint_8/log_duration.h
        src/sprint_8/log_duration.h
        src/sprint_8/process_queries.h
        src/sprint_8/concurent_map.h
        src/sprint_9/geo.h
        src/sprint_9/input_reader.cpp
        src/sprint_9/input_reader.h
        src/sprint_9/stat_reader.cpp
        src/sprint_9/stat_reader.h
        src/sprint_9/transport_catalogue.cpp
        src/sprint_9/transport_catalogue.h
        src/sprint_10/json.cpp
        src/sprint_10/json.h
        src/sprint_10/domain.cpp
        src/sprint_10/domain.h
        src/sprint_10/geo.cpp
        src/sprint_10/geo.h
        src/sprint_10/json_reader.cpp
        src/sprint_10/json_reader.h
        src/sprint_10/map_renderer.cpp
        src/sprint_10/map_renderer.h
        src/sprint_10/request_handler.cpp
        src/sprint_10/request_handler.h
        src/sprint_10/transport_catalogue.cpp
        src/sprint_10/transport_catalogue.h)
