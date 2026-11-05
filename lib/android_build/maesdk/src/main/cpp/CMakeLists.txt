cmake_minimum_required(VERSION 3.15...3.31)
project(MaesdkAndroid LANGUAGES C CXX)

get_filename_component(SDK_ROOT
  "${CMAKE_CURRENT_LIST_DIR}/../../../../../.." ABSOLUTE)

set(BUILD_SHARED_LIBS ON CACHE BOOL "")
set(MATSDK_BUILD_JNI_WRAPPER ON CACHE BOOL "" FORCE)
if(NOT DEFINED MATSDK_ANDROID_USE_ROOM AND NOT DEFINED USE_ROOM)
  set(MATSDK_ANDROID_USE_ROOM ON CACHE BOOL "Use Android Room for offline storage")
endif()
set(MATSDK_BUILD_PACKAGE OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_FUNC_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory("${SDK_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/matsdk")

# Preserve the Java/AAR runtime name: System.loadLibrary("maesdk").
set_target_properties(mat PROPERTIES OUTPUT_NAME maesdk)
