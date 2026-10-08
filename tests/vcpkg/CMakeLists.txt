cmake_minimum_required(VERSION 3.15...3.31)
project(cpp-client-telemetry_vcpkg_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Match vcpkg static triplet CRT linkage
if(MSVC AND VCPKG_TARGET_TRIPLET MATCHES "-static$")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

option(MATSDK_TEST_PRELOAD_DEPENDENCIES
  "Supply canonical targets required by a caller-provided static SDK package" OFF)
if(MATSDK_TEST_PRELOAD_DEPENDENCIES)
  find_package(CURL REQUIRED)
  find_package(SQLite3 REQUIRED)
  if(NOT TARGET SQLite3::SQLite3)
    add_library(SQLite3::SQLite3 INTERFACE IMPORTED)
    set_property(TARGET SQLite3::SQLite3 PROPERTY INTERFACE_LINK_LIBRARIES SQLite::SQLite3)
  endif()
  find_package(ZLIB REQUIRED)
endif()

find_package(MSTelemetry CONFIG REQUIRED)

get_target_property(SDK_COMPILE_OPTIONS MSTelemetry::mat INTERFACE_COMPILE_OPTIONS)
get_target_property(SDK_COMPILE_DEFINITIONS MSTelemetry::mat INTERFACE_COMPILE_DEFINITIONS)
if(SDK_COMPILE_OPTIONS MATCHES "fno-exceptions|/EHs-c-"
    OR SDK_COMPILE_DEFINITIONS MATCHES "_HAS_EXCEPTIONS|MATSDK_DISABLE_LOGGING|MATSDK_DISABLE_DEVICE_ID")
  message(FATAL_ERROR "SDK-private compiler policy leaked to consumers.")
endif()

add_executable(vcpkg_test main.cpp)
target_link_libraries(vcpkg_test PRIVATE MSTelemetry::mat)

enable_testing()
add_test(NAME vcpkg_integration_test COMMAND vcpkg_test)
