#######################################################################################################
# Generates project files for Swift wrapper files and generates executable based on the main.swift file.
#
# Dependecies:
# Please build Obj-c dependencies required by Swift by running
# ./build.sh clean release
# in the source folder to generate the required Obj-C library. It should be installed at /usr/local/lib
#######################################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

# Set build and path related variables
string(TOLOWER ${CMAKE_BUILD_TYPE} LOWER_BUILD_TYPE)
set(PROJECT_NAME "OneDSSwift")
set(SWIFT_BINARY_OUTPUT_FOLDER ".build/${LOWER_BUILD_TYPE}/${PROJECT_NAME}.build") # Swift object file output path: .build/debug/OneDSSwift.build/*.o

PROJECT(${PROJECT_NAME})
enable_language(Swift)

# Set flags and paths
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS} -import-objc-header ObjCModule-Bridging-Header.h")
set(INSTALLED_LIB_PATH "/usr/local/lib")

# Add system libs
find_library(SYSTEM_CONFIGURATION SystemConfiguration)
find_package(ZLIB REQUIRED)
find_library(NETWORK_FRAMEWORK Network)

# Add libmat.a library which has Obj-C dependencies built needed by Swift
find_library(MAT_LIB libmat.a PATHS ${INSTALLED_LIB_PATH})
find_library(SQLITE3_LIB libsqlite3.a PATH ${INSTALLED_LIB_PATH})

# Run swift build to compile and generate object files for swift
execute_process(
    COMMAND swift build -c ${LOWER_BUILD_TYPE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE SWIFT_BUILD_RESULT
)

if(SWIFT_BUILD_RESULT EQUAL 0)
    message("Swift build succeeded!")

    # Combine all the generated object files from swift code into static lib: lib onedsswift .a
    # swift build does not generate a single static lib file
    file(GLOB SWIFT_OBJECT_FILES "${SWIFT_BINARY_OUTPUT_FOLDER}/*.o")
    message(${SWIFT_BINARY_OUTPUT_FOLDER})
    message(${SWIFT_OBJECT_FILES})
    # add_library(onedsswift STATIC ${SWIFT_OBJECT_FILES})
    add_custom_command(
        OUTPUT libonedsswift.a
        COMMAND ar rcs libonedswift.a ${SWIFT_OBJECT_FILES}
        DEPENDS ${SWIFT_OBJECT_FILES}
        COMMENT "Creating static lib: libonedsswift.a"
    )

    if(EXISTS )
        message("Swift wrapper static lib created at: ")
    else()
        message(FATAL_ERROR, "Swift wrapper static lib creation failed.")
    endif()
else()
    message(FATAL_ERROR, "Swift build failed with error code: ${SWIFT_BUILD_RESULT}")
endif()

if(NOT SQLITE3_LIB)
    set(SQLITE3_LIB "sqlite3")
endif()

if(NOT MAT_LIB)
    message(FATAL_ERROR, "libmat.a not found at /usr/local/lib. Please run ./build.sh at root and install libmat.a.")
endif()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()

# Add source files to the target
file(GLOB SWIFT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.swift")
add_executable(swift_sample ${SWIFT_FILES})

target_link_libraries(swift_sample
    ${MAT_LIB}
    ${SYSTEM_CONFIGURATION}
    ${SQLITE3_LIB}
    stdc++
    ZLIB::ZLIB
    ${NETWORK_FRAMEWORK})
