cmake_minimum_required(VERSION 3.1.0)
project(MacProxy)

# Uncomment for building i386 binary on x86_64 system
#set(CMAKE_SYSTEM_PROCESSOR i386)

# For ARM / Raspberry Pi 3 cross-compile
# set(MAT_SDK_LIB       /usr/local/lib/armv7l-linux-gnu)

# Point example to SDK dirs for x86_64 Desktop
if(EXISTS "/usr/local/lib/libmat.a")
# Use local libmat.a
set(MAT_SDK_LIB	/usr/local/lib/)
else()
# Use architecture-specific libmat.a
set(MAT_SDK_LIB	/usr/local/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
endif()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -gdwarf-2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -gdwarf-2 -std=c++11")

find_package (Threads)

set(MAT_SDK_INCLUDE	/usr/local/include/mat)

# 1DS SDK to include dirs
include_directories( . ${MAT_SDK_INCLUDE} )

# Link main.cpp to executable
add_executable(MacProxy main.cpp HttpEventListener.cpp)
source_group(" " REGULAR_EXPRESSION "")

# Prefer linking to more recent local sqlite3
if(EXISTS "/usr/local/lib/libsqlite3.a")
set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
else()
set (SQLITE3_LIB "sqlite3")
endif()

set (PLATFORM_LIBS "")
# Add flags for obtaining system UUID via IOKit
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set (PLATFORM_LIBS "-framework CoreFoundation -framework IOKit -framework CFNetwork")
endif()

# Raspberry Pi 4 with gcc-8 on ARMv7l requires -latomic
if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
  set (PLATFORM_LIBS "atomic")
endif()

target_link_libraries(MacProxy ${MAT_SDK_LIB}/libmat.a curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} ${PLATFORM_LIBS} dl)
