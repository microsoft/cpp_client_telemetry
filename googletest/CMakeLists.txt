cmake_minimum_required(VERSION 2.6.4)

# If building for iOS, set all the iOS options
if(BUILD_IOS)
  set(TARGET_ARCH "APPLE")
  set(IOS True)
  set(APPLE True)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "Force unset of the deployment target for iOS" FORCE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -miphoneos-version-min=10.0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -miphoneos-version-min=10.0 -std=c++11")
  set(IOS_PLATFORM "iphonesimulator")
  set(CMAKE_SYSTEM_PROCESSOR x86_64)
  execute_process(COMMAND xcodebuild -version -sdk ${IOS_PLATFORM} Path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message("-- CMAKE_OSX_SYSROOT       ${CMAKE_OSX_SYSROOT}")
elseif(${ARCH} STREQUAL "arm64")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
  set(CMAKE_SYSTEM_PROCESSOR arm64)
  set(APPLE True)
endif()

project( googletest-distribution )

enable_testing()

option(BUILD_GTEST "Builds the googletest subproject" OFF)

#Note that googlemock target already builds googletest
option(BUILD_GMOCK "Builds the googlemock subproject" ON)

if(BUILD_GMOCK)
  add_subdirectory( googlemock )
elseif(BUILD_GTEST)
  add_subdirectory( googletest )
endif()
