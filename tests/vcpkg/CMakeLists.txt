cmake_minimum_required(VERSION 3.15)
project(mstelemetry_vcpkg_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Match vcpkg static triplet CRT linkage
if(MSVC AND VCPKG_TARGET_TRIPLET MATCHES "-static$")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

find_package(MSTelemetry CONFIG REQUIRED)

add_executable(vcpkg_test main.cpp)
target_link_libraries(vcpkg_test PRIVATE MSTelemetry::mat)

enable_testing()
add_test(NAME vcpkg_integration_test COMMAND vcpkg_test)
