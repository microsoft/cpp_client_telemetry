cmake_minimum_required(VERSION 3.5)
project(SampleCpp)

# Uncomment for building i386 binary on x86_64 system
#set(CMAKE_SYSTEM_PROCESSOR i386)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/MSTelemetrySample.cmake)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -gdwarf-2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -gdwarf-2 -std=c++11")

find_package (Threads)

# 1DS SDK to include dirs
include_directories(. ${MATSDK_SAMPLE_INCLUDE_DIRS})

# Link main.cpp to executable
add_executable(SampleCpp main.cpp DebugCallback.cpp demo.c)
source_group(" " REGULAR_EXPRESSION "")

set (PLATFORM_LIBS "")
# The 1DS SDK's required Apple frameworks are provided by MATSDK_SAMPLE_PLATFORM_LIBS.

# Raspberry Pi 4 with gcc-8 on ARMv7l requires -latomic
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
  set (PLATFORM_LIBS "atomic")
endif()

#tcmalloc turned off by default
#target_link_libraries(SampleCpp ${MAT_SDK_LIB}/libmat.a curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} dl tcmalloc)

# TODO: use add_library to allow linking against a proper exported SDK target
target_link_libraries(SampleCpp ${MATSDK_LIBRARY} curl z ${CMAKE_THREAD_LIBS_INIT} ${MATSDK_SQLITE3_LIB} ${MATSDK_SAMPLE_PLATFORM_LIBS} ${PLATFORM_LIBS} dl)
