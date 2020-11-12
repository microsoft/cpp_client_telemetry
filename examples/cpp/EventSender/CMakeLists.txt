cmake_minimum_required(VERSION 3.1.0)
project(EventSender)

# Uncomment for building i386 binary on x86_64 system
#set(CMAKE_SYSTEM_PROCESSOR i386)

# For ARM / Raspberry Pi 3 cross-compile
# set(MATSDK_LIB       /usr/local/lib/armv7l-linux-gnu/libmat.a)

# Point example to SDK dirs for x86_64 Desktop

set (PLATFORM_LIBS "")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -gdwarf-2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -gdwarf-2 -std=c++11")
find_package (Threads)

# Link main.cpp to executable
add_executable(EventSender EventSender.cpp)
source_group(" " REGULAR_EXPRESSION "")

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
  include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.Android.txt)
else()
  if(EXISTS "/usr/local/lib/libmat.a")
  # Use local libmat.a
    set(MATSDK_LIB	/usr/local/lib/libmat.a)
  else()
  # Use architecture-specific libmat.a
    set(MATSDK_LIB	/usr/local/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/libmat.a)
  endif()

  set(MAT_SDK_INCLUDE	/usr/local/include/mat)

  # Prefer linking to more recent local sqlite3
  if(EXISTS "/usr/local/lib/libsqlite3.a")
    set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
  else()
    set (SQLITE3_LIB "sqlite3")
  endif()

  # Add flags for obtaining system UUID via IOKit
  if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set (PLATFORM_LIBS "-framework CoreFoundation -framework IOKit")
  endif()
  target_link_libraries(EventSender ${MATSDK_LIB} curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} ${PLATFORM_LIBS} dl)
endif()

# 1DS SDK to include dirs
include_directories( . ${MAT_SDK_INCLUDE} )

#tcmalloc turned off by default
#target_link_libraries(EventSender ${MATSDK_LIB} curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} dl tcmalloc)
