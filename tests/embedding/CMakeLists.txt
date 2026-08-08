cmake_minimum_required(VERSION 3.15...3.31)
project(cpp-client-telemetry_embedding_test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
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
  find_path(MATSDK_TEST_CURL_INCLUDE_DIR curl/curl.h)
  find_library(MATSDK_TEST_CURL_LIBRARY NAMES curl)
  if(NOT MATSDK_TEST_CURL_INCLUDE_DIR OR NOT MATSDK_TEST_CURL_LIBRARY)
    message(FATAL_ERROR "System curl headers/library were not found.")
  endif()
  add_library(matsdk_test_curl INTERFACE)
  target_include_directories(matsdk_test_curl INTERFACE
    "${MATSDK_TEST_CURL_INCLUDE_DIR}")
  target_link_libraries(matsdk_test_curl INTERFACE
    "${MATSDK_TEST_CURL_LIBRARY}")
  add_library(CURL::libcurl ALIAS matsdk_test_curl)
  set(CMAKE_DISABLE_FIND_PACKAGE_CURL ON CACHE BOOL "" FORCE)
endif()

option(MATSDK_EMBEDDING_PRELOAD_STORAGE_DEPS
  "Pre-create SQLite::SQLite3/ZLIB::ZLIB and disable later discovery" OFF)
if(MATSDK_EMBEDDING_PRELOAD_STORAGE_DEPS)
  find_path(MATSDK_TEST_SQLITE_INCLUDE_DIR sqlite3.h)
  find_library(MATSDK_TEST_SQLITE_LIBRARY NAMES sqlite3)
  find_path(MATSDK_TEST_ZLIB_INCLUDE_DIR zlib.h)
  find_library(MATSDK_TEST_ZLIB_LIBRARY NAMES z zlib)
  if(NOT MATSDK_TEST_SQLITE_INCLUDE_DIR OR NOT MATSDK_TEST_SQLITE_LIBRARY
     OR NOT MATSDK_TEST_ZLIB_INCLUDE_DIR OR NOT MATSDK_TEST_ZLIB_LIBRARY)
    message(FATAL_ERROR "System SQLite/zlib headers or libraries were not found.")
  endif()
  add_library(matsdk_test_sqlite INTERFACE)
  target_include_directories(matsdk_test_sqlite INTERFACE
    "${MATSDK_TEST_SQLITE_INCLUDE_DIR}")
  target_link_libraries(matsdk_test_sqlite INTERFACE
    "${MATSDK_TEST_SQLITE_LIBRARY}")
  add_library(SQLite::SQLite3 ALIAS matsdk_test_sqlite)
  add_library(matsdk_test_zlib INTERFACE)
  target_include_directories(matsdk_test_zlib INTERFACE
    "${MATSDK_TEST_ZLIB_INCLUDE_DIR}")
  target_link_libraries(matsdk_test_zlib INTERFACE
    "${MATSDK_TEST_ZLIB_LIBRARY}")
  add_library(ZLIB::ZLIB ALIAS matsdk_test_zlib)
  set(CMAKE_DISABLE_FIND_PACKAGE_SQLite3 ON CACHE BOOL "" FORCE)
  set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON CACHE BOOL "" FORCE)
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
