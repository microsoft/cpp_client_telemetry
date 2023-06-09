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
set(SWIFT_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.build/${LOWER_BUILD_TYPE}") # By default .build is the output folder of `swift build`
set(SWIFT_OBJECT_OUTPUT_FOLDER "${SWIFT_MODULE_PATH}/${PROJECT_NAME}.build") # Swift object file output path: .build/debug/OneDSSwift.build/*.o
set(SWIFT_STATIC_LIB "${CMAKE_BINARY_DIR}/StaticLibs/libonedsswift.a") # Putting newly created static libs in this folder
set(OBJC_MODULE_DIROBJC_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Modules")

PROJECT(${PROJECT_NAME})
enable_language(Swift)

# Set flags and paths
set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS} -import-objc-header ObjCModule-Bridging-Header.h")
set(INSTALLED_LIB_PATH "/usr/local/lib")

# Add system libs
find_library(SYSTEM_CONFIGURATION SystemConfiguration)
find_package(ZLIB REQUIRED)
find_library(NETWORK_FRAMEWORK Network)

# Add libmat.a library which has Obj-C dependencies built needed by Swift wrappers
find_library(MAT_LIB libmat.a PATHS ${INSTALLED_LIB_PATH})
find_library(SQLITE3_LIB libsqlite3.a PATH ${INSTALLED_LIB_PATH})

# Check and bail out if required libs don't exist
if(NOT SQLITE3_LIB)
    set(SQLITE3_LIB "sqlite3")
endif()

if(NOT MAT_LIB)
    message(FATAL_ERROR, "libmat.a not found at /usr/local/lib. Please run ./build.sh at root and install libmat.a.")
endif()

# --------------------------------------------
# Swift wrappers: Build and generate static lib
# --------------------------------------------

# Run swift build to compile and generate object files for swift
message("\nRunning swift build...")
execute_process(
    COMMAND swift build -c ${LOWER_BUILD_TYPE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE SWIFT_BUILD_RESULT
)

if(SWIFT_BUILD_RESULT EQUAL 0)
    message("Swift build succeeded!")

    # Combine all the generated object files from swift code into static lib: lib onedsswift .a
    # swift build does not generate a single static lib file
    file(GLOB SWIFT_OBJECT_FILES "${SWIFT_OBJECT_OUTPUT_FOLDER}/*.o")
    execute_process(
        COMMAND ar rcs ${SWIFT_STATIC_LIB} ${SWIFT_OBJECT_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    if(EXISTS ${SWIFT_STATIC_LIB})
        message("Swift wrapper static lib created at: ${SWIFT_STATIC_LIB}")
    else()
        message(FATAL_ERROR, "Swift wrapper static lib creation failed.")
    endif()

else()
    message(FATAL_ERROR, "Swift build failed with error code: ${SWIFT_BUILD_RESULT}")
endif()

# --------------------------
# End: Swift wrappers build
# --------------------------

# -------------------------------------------
# SwiftC: Invoke and generate sample_app exe
# -------------------------------------------

# Add swift files (outside of swift package) to the target
file(GLOB SWIFT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Sample/main.swift")

# Collect all libs to be linked into single variable
set(ADDITIONAL_LIBS
    ${MAT_LIB} # libmat.a contains ObjC symbols
    ${SYSTEM_CONFIGURATION}
    ${SQLITE3_LIB}
    ${SWIFT_STATIC_LIB} # libonedsswift.a contains swift wrapper symbols
    stdc++
    ZLIB::ZLIB
    ${NETWORK_FRAMEWORK}
)

# Collect all the modules to be included
set(INCLUDE_MODULES
    ${OBJC_MODULE_DIR} # Needed for Enums and others declared in ObjC headers
    ${SWIFT_MODULE_PATH} # Swift wrappers module OneDSSwift, after the build
)

# Invoke swiftc to create the exe
message("\nRunning swiftc...")
execute_process(
    COMMAND swiftc -o ${CMAKE_BINARY_DIR}/sample_app ${SWIFT_FILES} -I ${INCLUDE_MODULES} -Xlinker ${ADDITIONAL_LIBS}
    RESULT_VARIABLE SWIFTC_RESULT
)

if(EXISTS ${CMAKE_BINARY_DIR}/sample_app)
    message("Executable is created at: ${CMAKE_BINARY_DIR}/sample_app")
else()
    message(FATALError, "Executable creation failed.")
endif()
