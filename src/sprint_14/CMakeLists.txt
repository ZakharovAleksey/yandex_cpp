cmake_minimum_required(VERSION 3.20)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "D:/education/cpp/libraries/protobuf/build_mingw/protobuf")
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

if (MSVC)
    message(STATUS "USE VISUAL STUDIO COMPILER")
    # All flags: https://docs.microsoft.com/ru-ru/cpp/build/reference/compiler-options-listed-by-category?view=msvc-160
    add_compile_options(/WX)
else ()
    message(STATUS "USE MINGW COMPILER")
    set(SYSTEM_LIBS -lstdc++)
endif ()


set(SPRINT_14_DIR src)
set(SPRINT_14_FILES
        ${SPRINT_14_DIR}/svg.cpp ${SPRINT_14_DIR}/svg.h
        ${SPRINT_14_DIR}/json.cpp ${SPRINT_14_DIR}/json.h
        ${SPRINT_14_DIR}/domain.cpp ${SPRINT_14_DIR}/domain.h
        ${SPRINT_14_DIR}/geo.cpp ${SPRINT_14_DIR}/geo.h
        ${SPRINT_14_DIR}/json_reader.cpp ${SPRINT_14_DIR}/json_reader.h
        ${SPRINT_14_DIR}/map_renderer.cpp ${SPRINT_14_DIR}/map_renderer.h
        ${SPRINT_14_DIR}/request_handler.cpp ${SPRINT_14_DIR}/request_handler.h
        ${SPRINT_14_DIR}/transport_catalogue.cpp ${SPRINT_14_DIR}/transport_catalogue.h
        ${SPRINT_14_DIR}/json_builder.cpp ${SPRINT_14_DIR}/json_builder.h
        ${SPRINT_14_DIR}/transport_router.cpp ${SPRINT_14_DIR}/transport_router.h
        ${SPRINT_14_DIR}/serialization.cpp ${SPRINT_14_DIR}/serialization.h
        ${SPRINT_14_DIR}/graph.h
        ${SPRINT_14_DIR}/ranges.h
        ${SPRINT_14_DIR}/router.h
        ${SPRINT_14_DIR}/canvas.h)

add_executable(sprint_14 transport_catalogue_main.cpp ${SPRINT_14_FILES})
protobuf_generate(LANGUAGE cpp TARGET sprint_14 PROTOS
        transport_catalogue.proto
        svg.proto
        map_renderer.proto
        graph.proto
        transport_router.proto)

target_include_directories(sprint_14 PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(sprint_14 PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(sprint_14 "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads ${SYSTEM_LIBS})