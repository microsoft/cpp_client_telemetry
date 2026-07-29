cmake_minimum_required(VERSION 3.15...3.31)
project(cpp-client-telemetry_embedding_test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MATSDK_LIBRARY_TYPE STATIC CACHE STRING "")
set(MATSDK_BUILD_HEADERS ON CACHE BOOL "" FORCE)
set(MATSDK_BUILD_LIBRARY ON CACHE BOOL "" FORCE)
set(MATSDK_BUILD_TEST_TOOL OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_FUNC_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_PACKAGE OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_OBJC_WRAPPER OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_SWIFT_WRAPPER OFF CACHE BOOL "" FORCE)
set(MATSDK_WARNINGS_AS_ERRORS ON CACHE BOOL "" FORCE)

option(MATSDK_EMBEDDING_PRELOAD_CURL
  "Pre-create CURL::libcurl and disable subsequent package discovery" OFF)
if(MATSDK_EMBEDDING_PRELOAD_CURL)
  find_package(CURL REQUIRED)
  # A later find_package(CURL REQUIRED) fails when this switch is enabled, so
  # this configuration proves the SDK consumes the pre-created target directly.
  set(CMAKE_DISABLE_FIND_PACKAGE_CURL ON CACHE BOOL "" FORCE)
endif()

option(MATSDK_EMBEDDING_USE_FETCHCONTENT
  "Exercise local-source FetchContent instead of add_subdirectory" OFF)
if(MATSDK_EMBEDDING_USE_FETCHCONTENT)
  include(FetchContent)
  FetchContent_Declare(cpp_client_telemetry
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
  FetchContent_MakeAvailable(cpp_client_telemetry)
else()
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." cpp_client_telemetry)
endif()

add_executable(embedding_test "${CMAKE_CURRENT_LIST_DIR}/../vcpkg/main.cpp")
target_link_libraries(embedding_test PRIVATE MSTelemetry::mat)
if(MSVC)
  target_compile_options(embedding_test PRIVATE /W4 /WX)
else()
  target_compile_options(embedding_test PRIVATE -Wall -Wextra -Werror)
endif()
