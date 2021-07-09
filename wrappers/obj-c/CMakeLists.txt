CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)

PROJECT(ObjC_Wrapper)

message("${CMAKE_CURRENT_SOURCE_DIR}")

#clang -L/build/Aria.SDK.Cpp/lib/include/public -L/build/Aria.SDK.Cpp/lib/include/mat -std=c++11 -stdlib=libc++ -framework Foundation -lc++ -o prog1 main.mm

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -gdwarf-2 -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -gdwarf-2 -std=c++11 -stdlib=libc++")
option(BUILD_PRIVACYGUARD "Build Privacy Guard"     YES)

# Point example to SDK dirs for x86_64 Desktop
if(EXISTS "/usr/local/lib/libmat.a")
# Use local libmat.a
set(MATSDK_LIB	/usr/local/lib/)
else()
# Use architecture-specific libmat.a
set(MATSDK_LIB	/usr/local/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
endif()

# Prefer linking to more recent local sqlite3
if(EXISTS "/usr/local/lib/libsqlite3.a")
set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
else()
set (SQLITE3_LIB "sqlite3")
endif()

set (PLATFORM_LIBS "")
# Add flags for obtaining system UUID via IOKit
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
set (PLATFORM_LIBS "-framework Foundation -framework CoreFoundation -framework IOKit -framework Network")
endif()

include_directories( 
    ../../lib/include/public/
    ../../lib/include/mat/
    )

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()

set(PLATFORM_FILES main.mm ODWLogManager.mm ODWLogger.mm ODWEventProperties.mm ODWLogConfiguration.mm ODWSemanticContext.mm)
add_executable( sample ${PLATFORM_FILES} )
target_link_libraries(sample ${MATSDK_LIB}/libmat.a curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} ${PLATFORM_LIBS} dl)

#SET( CMAKE_EXE_LINKER_FLAGS "-lmat -framework Foundation -w -lc++" )
