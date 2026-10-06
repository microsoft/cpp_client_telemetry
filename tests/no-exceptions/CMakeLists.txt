cmake_minimum_required(VERSION 3.15...3.31)
project(MatsdkNoExceptionsSmoke LANGUAGES C CXX)

set(MATSDK_DISABLE_EXCEPTIONS ON CACHE BOOL
  "Compile SDK-owned C++ targets without exception support")
set(MATSDK_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_FUNC_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_TEST_TOOL OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_PACKAGE OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_SWIFT_WRAPPER OFF CACHE BOOL "" FORCE)
add_subdirectory(../.. sdk)

add_executable(no-exceptions-smoke no-exceptions-smoke.cpp)
target_compile_features(no-exceptions-smoke PRIVATE cxx_std_14)
target_include_directories(no-exceptions-smoke PRIVATE
  ../../lib
  ../../lib/include
  ../../lib/include/mat
  ../../lib/pal
  ../../lib/utils)
target_link_libraries(no-exceptions-smoke PRIVATE mat matsdk_internal_config matsdk_build_options)
enable_testing()
add_test(NAME no-exceptions-smoke COMMAND no-exceptions-smoke)
