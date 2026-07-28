cmake_minimum_required(VERSION 3.15...3.31)
project(cpp-client-telemetry_embedding_test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_HEADERS ON CACHE BOOL "" FORCE)
set(BUILD_LIBRARY ON CACHE BOOL "" FORCE)
set(BUILD_TEST_TOOL OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_FUNC_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_PACKAGE OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." cpp_client_telemetry)

add_executable(embedding_test "${CMAKE_CURRENT_LIST_DIR}/../vcpkg/main.cpp")
target_link_libraries(embedding_test PRIVATE MSTelemetry::mat)
