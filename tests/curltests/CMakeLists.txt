cmake_minimum_required(VERSION 3.5)
project(curltests)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -std=c++11")

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out)

################################
# GTest
################################
ADD_SUBDIRECTORY (/usr/src/gtest out)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

################################
# Main executable
################################
add_executable(http-post main.cpp)
source_group(" " REGULAR_EXPRESSION "")
target_link_libraries(http-post curl gtest gtest_main)
