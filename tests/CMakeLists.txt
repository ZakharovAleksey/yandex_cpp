project(google_tests)

add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)

add_executable(google_tests test_string_processing.cpp ../src/sprint_5/document.cpp ../src/sprint_5/document.h ../src/sprint_5/paginator.h ../src/sprint_5/request_queue.cpp ../src/sprint_5/request_queue.h ../src/sprint_5/search_server.cpp ../src/sprint_5/search_server.h ../src/sprint_5/string_processing.cpp ../src/sprint_5/string_processing.h ../src/sprint_5/search_server.cpp ../src/sprint_5/search_server.h ../src/sprint_5/log_duration.h ../src/sprint_5/log_duration.h utils/test_utils.cpp utils/test_utils.h test_paginator.cpp test_log_duration.cpp test_request_queue.cpp test_search_server.cpp)
target_link_libraries(google_tests gtest gtest_main)
