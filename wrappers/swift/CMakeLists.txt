#######################################################################################################
# Generates project files for Swift wrapper files and generates executable based on the main.swift file.
#
# Dependecies:
# Please build Obj-c dependencies required by Swift by running
# ./build.sh clean release
# in the source folder to generate the required Obj-C library. It should be installed at /usr/local/lib
#######################################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)

PROJECT(Swift_Wrapper)
message("${CMAKE_CURRENT_SOURCE_DIR}")
enable_language(Swift)

# Set flags and paths
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS} -import-objc-header Swift_Wrapper-Bridging-Header.h")
set(INSTALLED_LIB_PATH "/usr/local/lib")

# Add system libs
find_library(SYSTEM_CONFIGURATION SystemConfiguration)
find_package(ZLIB REQUIRED)
find_library(NETWORK_FRAMEWORK Network)

# Add libmat.a library which has Obj-C dependencies built needed by Swift
find_library(MAT_LIB libmat.a PATHS ${INSTALLED_LIB_PATH})
find_library(SQLITE3_LIB libsqlite3.a PATH ${INSTALLED_LIB_PATH})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()

# Add source files to the target
file(GLOB SWIFT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.swift")
#set(PLATFORM_FILES main.swift SDWEventProperties.swift SDWLogger.swift SDWLogManager.swift)
add_executable(swift_sample ${SWIFT_FILES})

target_link_libraries(swift_sample
    ${MAT_LIB}
    ${SYSTEM_CONFIGURATION}
    ${SQLITE3_LIB}
    stdc++
    ZLIB::ZLIB
    ${NETWORK_FRAMEWORK})
