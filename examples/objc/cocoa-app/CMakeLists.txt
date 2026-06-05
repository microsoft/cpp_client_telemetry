cmake_minimum_required(VERSION 3.5)
project(foo)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/MSTelemetrySample.cmake)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -gdwarf-2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -gdwarf-2 -std=c++11")

find_package (Threads)

# 1DS SDK to include dirs
include_directories(. ${MATSDK_SAMPLE_INCLUDE_DIRS})

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

target_link_libraries(foo ${MATSDK_LIBRARY} curl z ${CMAKE_THREAD_LIBS_INIT} ${MATSDK_SQLITE3_LIB} ${MATSDK_SAMPLE_PLATFORM_LIBS} ${PLATFORM_LIBS} dl)
