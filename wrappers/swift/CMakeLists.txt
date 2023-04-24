CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

PROJECT(Swift_Wrapper)

message("${CMAKE_CURRENT_SOURCE_DIR}")

enable_language(Swift)

# TODO: add logic to build dependencies and stuff

include_directories(
    ../../lib/include/public/
    ../../lib/include/mat/
)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()

# Add source files to the target
set(PLATFORM_FILES main.swift)
add_executable(swift_sample ${PLATFORM_FILES})

# Set compiler and options
# set(CMAKE_GENERATOR Xcode)
# set(CMAKE_Swift_COMPILER "/usr/bin/swift")
# #set(CMAKE_Swift_LINK_EXECUTABLE "/usr/bin/swiftc")
# set_target_properties(swift_sample PROPERTIES
#     LINKER_LANGUAGE Swift
#     SWIFT_VERSION 5
#     XCODE_ATTRIBUTE_SWIFT_VERSION "5"
#     XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES"
# )
# set(CMAKE_XCODE_GENERATE_SCHEME ON)

target_link_libraries(swift_sample)
