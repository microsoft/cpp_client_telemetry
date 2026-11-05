cmake_minimum_required(VERSION 3.19...3.31)
project(AndroidHttpClientTests LANGUAGES C CXX)

if(NOT ANDROID)
  message(FATAL_ERROR "These JNI lifetime tests must run on Android ART.")
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_JNI_WRAPPER OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_FUNC_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_TEST_TOOL OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_PACKAGE OFF CACHE BOOL "" FORCE)
add_subdirectory(../.. sdk)

add_library(android-http-client-tests SHARED android-http-client-tests.cpp)
target_compile_features(android-http-client-tests PRIVATE cxx_std_14)
target_include_directories(android-http-client-tests PRIVATE
  ../../lib
  ../../lib/include
  ../../lib/include/mat
  ../../lib/pal
  ../../lib/utils)
target_link_libraries(android-http-client-tests PRIVATE
  mat matsdk_internal_config matsdk_build_options)
