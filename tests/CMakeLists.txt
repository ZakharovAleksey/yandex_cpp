project(google_tests)

add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)

add_executable(google_tests
        ../src/sprint_6/single_linked_list.h
        ../src/sprint_8/document.cpp
        ../src/sprint_8/document.h
        ../src/sprint_8/paginator.h
        ../src/sprint_8/request_queue.cpp
        ../src/sprint_8/request_queue.h
        ../src/sprint_8/search_server.cpp
        ../src/sprint_8/search_server.h
        ../src/sprint_8/string_processing.cpp
        ../src/sprint_8/string_processing.h
        ../src/sprint_8/search_server.cpp
        ../src/sprint_8/search_server.h
        ../src/sprint_8/log_duration.h
        ../src/sprint_8/log_duration.h
        ../src/sprint_9/geo.h
        ../src/sprint_9/input_reader.h
        ../src/sprint_9/input_reader.cpp
        ../src/sprint_9/stat_reader.h
        ../src/sprint_9/stat_reader.cpp
        ../src/sprint_9/transport_catalogue.h
        ../src/sprint_9/transport_catalogue.cpp
        test_log_duration.cpp
        test_paginator.cpp
        test_request_queue.cpp
        test_search_server.cpp
        test_simple_vector.cpp
        test_single_linked_list.cpp
        test_string_processing.cpp
        test_transport_catalogue.cpp
)

target_link_libraries(google_tests gtest gtest_main)
