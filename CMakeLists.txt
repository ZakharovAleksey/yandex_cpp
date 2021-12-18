cmake_minimum_required(VERSION 3.16)
project(yp_cpp1)

set(CMAKE_CXX_STANDARD 17)
#[[
Way to add flags: set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

Compilation flags:
    WSL: -fsanitize=address -g -fno-omit-frame-pointer -fno-optimize-sibling-calls -ltbb -lpthread
    VS: -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer -g -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_LIBCPP_DEBUG=1
]]

if (MSVC)
    message(STATUS "USE VISUAL STUDIO COMPILER")
    # All flags: https://docs.microsoft.com/ru-ru/cpp/build/reference/compiler-options-listed-by-category?view=msvc-160
    add_compile_options(/WX)
else ()
    message(STATUS "USE MINGW COMPILER")
    set(SYSTEM_LIBS -lstdc++)
#    add_compile_options(-Wall -Wextra -ltbb -lpthread) #   +fs --enable-libstdcxx-filesystem-ts
endif ()

add_subdirectory(tests)

set(SOURCE_DIR src)

# SPRINT 12
set(SPRINT_12_DIR ${SOURCE_DIR}/sprint_12)
set(SPRINT_12_FILES
        ${SPRINT_12_DIR}/svg.cpp ${SPRINT_12_DIR}/svg.h
        ${SPRINT_12_DIR}/json.cpp ${SPRINT_12_DIR}/json.h
        ${SPRINT_12_DIR}/domain.cpp ${SPRINT_12_DIR}/domain.h
        ${SPRINT_12_DIR}/geo.cpp ${SPRINT_12_DIR}/geo.h
        ${SPRINT_12_DIR}/json_reader.cpp ${SPRINT_12_DIR}/json_reader.h
        ${SPRINT_12_DIR}/map_renderer.cpp ${SPRINT_12_DIR}/map_renderer.h
        ${SPRINT_12_DIR}/request_handler.cpp ${SPRINT_12_DIR}/request_handler.h
        ${SPRINT_12_DIR}/transport_catalogue.cpp ${SPRINT_12_DIR}/transport_catalogue.h
        ${SPRINT_12_DIR}/json_builder.cpp ${SPRINT_12_DIR}/json_builder.h
        ${SPRINT_12_DIR}/transport_router.cpp ${SPRINT_12_DIR}/transport_router.h
        ${SPRINT_12_DIR}/graph.h
        ${SPRINT_12_DIR}/ranges.h
        ${SPRINT_12_DIR}/router.h
        ${SPRINT_12_DIR}/canvas.h)

# SPRINT 13
set(SPRINT_13_DIR ${SOURCE_DIR}/sprint_13)
set(SPRINT_13_FILES ${SPRINT_13_DIR}/optional.h ${SPRINT_13_DIR}/vector.h)

# SPRINT 15
set(SPRINT_15_DIR ${SOURCE_DIR}/sprint_15)
set(SPRINT_15_FILES
        ${SPRINT_15_DIR}/lexer.h ${SPRINT_15_DIR}/lexer.cpp
        ${SPRINT_15_DIR}/runtime.h ${SPRINT_15_DIR}/runtime.cpp
        ${SPRINT_15_DIR}/parse.h ${SPRINT_15_DIR}/parse.cpp
        ${SPRINT_15_DIR}/statement.h ${SPRINT_15_DIR}/statement.cpp
        ${SPRINT_15_DIR}/test_runner_p.h
        ${SPRINT_15_DIR}/lexer_test_open.cpp
        ${SPRINT_15_DIR}/runtime_test.cpp
        ${SPRINT_15_DIR}/parse_test.cpp
        ${SPRINT_15_DIR}/statement_test.cpp)

# SET UP BUILD TARGETS
add_executable(sprint_12 main.cpp ${SPRINT_12_FILES})
add_executable(sprint_13 main.cpp ${SPRINT_13_FILES})
add_executable(sprint_15 main.cpp ${SPRINT_15_FILES})

