cmake_minimum_required(VERSION 3.1.0)
project(SampleCpp)

# Uncomment for building i386 binary on x86_64 system
#set(CMAKE_SYSTEM_PROCESSOR i386)

if (NOT TARGET_ARCH)
  set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

# For ARM / Raspberry Pi 3 cross-compile
# set(MAT_SDK_LIB       /usr/local/lib/armv7l-linux-gnu)

# Search for static lib first
find_path(MAT_SDK_LIB_PATH NAMES libmat.a PATHS /usr/lib /usr/local/lib /usr/local/lib/${TARGET_ARCH}-linux-gnu)
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Search for .dylib on Mac OS X
  find_path(MAT_SDK_DYLIB_PATH NAMES libmat.dylib PATHS /usr/lib /usr/local/lib)
endif()
find_library(LIBMAT NAMES libmat.a libmat.dylib HINTS ${MAT_SDK_LIB_PATH} ${MAT_SDK_DYLIB_PATH})

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -gdwarf-2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -gdwarf-2 -std=c++11")

find_package (Threads)

set(MAT_SDK_INCLUDE	/usr/local/include/mat)

# 1DS SDK to include dirs
include_directories( . ${MAT_SDK_INCLUDE} )

# Link main.cpp to executable
add_executable(SampleCpp main.cpp DebugCallback.cpp demo.c)
source_group(" " REGULAR_EXPRESSION "")

# NOTE: modify this sample if necessary to use vcpkg-provided sqlite library
# Prefer linking to more recent local sqlite3
if(EXISTS "/usr/local/lib/libsqlite3.a")
set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
else()
set (SQLITE3_LIB "sqlite3")
endif()

set (PLATFORM_LIBS "")
# Add flags for obtaining system UUID via IOKit
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set (PLATFORM_LIBS "-framework CoreFoundation -framework IOKit -framework Foundation")
endif()

# Raspberry Pi 4 with gcc-8 on ARMv7l requires -latomic
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
  set (PLATFORM_LIBS "atomic")
endif()

#tcmalloc turned off by default
#target_link_libraries(SampleCpp ${MAT_SDK_LIB}/libmat.a curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} dl tcmalloc)

# TODO: use add_library to allow linking against ${LIBMAT_STATIC}
target_link_libraries(SampleCpp ${LIBMAT} curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} ${PLATFORM_LIBS} dl)
