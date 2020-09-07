cmake_minimum_required(VERSION 2.8.8)
project(foo)

# Point example to SDK dirs for x86_64 Desktop
if(EXISTS "/usr/local/lib/libmat.a")
# Use local libmat.a
set(MATSDK_LIB	/usr/local/lib/)
else()
# Use architecture-specific libmat.a
set(MATSDK_LIB	/usr/local/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
endif()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -gdwarf-2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -gdwarf-2 -std=c++11")

find_package (Threads)

set(MATSDK_INCLUDE	/usr/local/include/mat)

# 1DS SDK to include dirs
include_directories( . ${MATSDK_INCLUDE} )

# Prefer linking to more recent local sqlite3
if(EXISTS "/usr/local/lib/libsqlite3.a")
set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
else()
set (SQLITE3_LIB "sqlite3")
endif()

set (PLATFORM_LIBS "")
# Add flags for obtaining system UUID via IOKit
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
set (PLATFORM_LIBS "-framework Foundation -framework CoreFoundation -framework IOKit -framework AppKit")
endif()


add_executable(
    foo
    MACOSX_BUNDLE
    AppDelegate.h
    AppDelegate.mm
    main.mm
    MainMenu.xib
)

set_source_files_properties(
    MainMenu.xib
    PROPERTIES
    MACOSX_PACKAGE_LOCATION
    Resources
)

set_target_properties(
    foo
    PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST
    ${CMAKE_CURRENT_LIST_DIR}/plist.in
)

target_link_libraries(foo ${MATSDK_LIB}/libmat.a curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} ${PLATFORM_LIBS} dl)
