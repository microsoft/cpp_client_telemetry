cmake_minimum_required(VERSION 3.15...3.31)
project(SampleCppMini)

# Uncomment for building i386 binary on x86_64 system
#set(CMAKE_SYSTEM_PROCESSOR i386)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/MSTelemetrySample.cmake)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -gdwarf-2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -gdwarf-2 -std=c++11")

find_package (Threads)

# 1DS SDK to include dirs
include_directories(. ${MATSDK_SAMPLE_INCLUDE_DIRS})

# Link main.cpp to executable
add_executable(SampleCppMini main.cpp demo.c)
source_group(" " REGULAR_EXPRESSION "")

# The 1DS SDK's required Apple frameworks are provided by MATSDK_SAMPLE_PLATFORM_LIBS.

#tcmalloc turned off by default
#target_link_libraries(SampleCppMini ${MATSDK_LIBRARY} curl z ${CMAKE_THREAD_LIBS_INIT} ${MATSDK_SQLITE3_LIB} ${MATSDK_SAMPLE_PLATFORM_LIBS} dl tcmalloc)

target_link_libraries(SampleCppMini ${MATSDK_LIBRARY} curl z ${CMAKE_THREAD_LIBS_INIT} ${MATSDK_SQLITE3_LIB} ${MATSDK_SAMPLE_PLATFORM_LIBS} dl)
