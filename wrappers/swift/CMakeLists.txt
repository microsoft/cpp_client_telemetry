CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

PROJECT(Swift_Wrapper)

message("${CMAKE_CURRENT_SOURCE_DIR}")

enable_language(Swift)

# TODO: add logic to build dependencies

include_directories(
    ../../lib/include/public/
    ../../lib/include/mat/
)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()

# Add source files to the target
set(PLATFORM_FILES main.swift EventProperties.swift)
set(CMAKE_Swift_FLAGS "${CMAKE_Swift_FLAGS} -import-objc-header Swift_Wrapper-Bridging-Header.h")
add_executable(swift_sample ${PLATFORM_FILES})
target_link_libraries(swift_sample)
