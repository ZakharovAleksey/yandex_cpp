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
        src/sprint_12/canvas.h
        src/sprint_12/svg.h
        src/sprint_12/svg.cpp
        src/sprint_12/json.cpp
        src/sprint_12/json.h
        src/sprint_12/domain.cpp
        src/sprint_12/domain.h
        src/sprint_12/geo.cpp
        src/sprint_12/geo.h
        src/sprint_12/json_reader.cpp
        src/sprint_12/json_reader.h
        src/sprint_12/map_renderer.cpp
        src/sprint_12/map_renderer.h
        src/sprint_12/request_handler.cpp
        src/sprint_12/request_handler.h
        src/sprint_12/transport_catalogue.cpp
        src/sprint_12/transport_catalogue.h
        src/sprint_12/json_builder.cpp
        src/sprint_12/json_builder.h
        src/sprint_12/graph.h
        src/sprint_12/ranges.h
        src/sprint_12/router.h
        src/sprint_12/transport_router.cpp
        src/sprint_12/transport_router.h
        )
