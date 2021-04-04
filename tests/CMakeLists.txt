project(google_tests)

add_subdirectory(lib/googletest)
include_directories(lib/googletest/googletest/include)

add_executable(google_tests testcase.cpp)
target_link_libraries(google_tests gtest gtest_main)
