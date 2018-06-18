cmake_minimum_required(VERSION 3.1.0)
project(sample1)

# Uncomment for building i386 binary on x86_64 system
#set(CMAKE_SYSTEM_PROCESSOR i386)

# For ARM / Raspberry Pi 3 cross-compile
# set(ARIA_SDK_LIB       /usr/local/lib/armv7l-linux-gnu)

# Point example to SDK dirs for x86_64 Desktop
set(ARIA_SDK_LIB	/usr/local/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -std=c++11")

find_package (Threads)

set(ARIA_SDK_INCLUDE	/usr/local/include/aria)

# Add SDK to include dirs
include_directories( . ${ARIA_SDK_INCLUDE} )

# Link main.cpp to executable
add_executable(sample1 main.cpp DebugCallback.cpp)
source_group(" " REGULAR_EXPRESSION "")

# Prefer linking to more recent local sqlite3
if(EXISTS "/usr/local/lib/libsqlite3.a")
set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
else()
set (SQLITE3_LIB "sqlite3")
endif()

#tcmalloc turned off
#target_link_libraries(sample1 ${ARIA_SDK_LIB}/libaria.a curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} dl tcmalloc)
target_link_libraries(sample1 ${ARIA_SDK_LIB}/libaria.a curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} dl)
