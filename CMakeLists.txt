cmake_minimum_required(VERSION 3.1.0)
project(MSTelemetry)

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

# Begin Uncomment for i386 build
#set(CMAKE_SYSTEM_PROCESSOR i386)
#set(CMAKE_C_FLAGS -m32)
#set(CMAKE_CXX_FLAGS -m32)
# End of i386 build

# We ask for 14, but we may get C++11 on older gcc-4.x and gcc-5.x
set(CMAKE_CXX_STANDARD 14)

if (NOT TARGET_ARCH)
  set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

# Enable ARC for obj-c on Apple
if(APPLE)
  message("-- BUILD_IOS:              ${BUILD_IOS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")

  # iOS build options
  option(BUILD_IOS           "Build for iOS"               NO)

  if(BUILD_IOS)
    set(TARGET_ARCH "APPLE")
    set(IOS True)
    set(APPLE True)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "Force unset of the deployment target for iOS" FORCE)
    if((${IOS_PLAT} STREQUAL "iphonesimulator") AND (${IOS_ARCH} STREQUAL "arm64"))
      # iOS arm64 simulator is supported starting BigSur
      # Unfortunately, CMAKE produces a device binary (not simulator) when providing -miphoneos-version-min flag when building iOS arm64 simulator
    else()
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -miphoneos-version-min=${IOS_DEPLOYMENT_TARGET}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -miphoneos-version-min=${IOS_DEPLOYMENT_TARGET}")
    endif()

    if((${IOS_PLAT} STREQUAL "iphoneos") OR (${IOS_PLAT} STREQUAL "iphonesimulator"))
      set(IOS_PLATFORM "${IOS_PLAT}")
    else()
      message(FATAL_ERROR "Unrecognized iOS platform '${IOS_PLAT}'")
    endif()

    if(${IOS_ARCH} STREQUAL "x86_64")
      set(IOS_PLATFORM "iphonesimulator")
      set(CMAKE_SYSTEM_PROCESSOR x86_64)
    elseif(${IOS_ARCH} STREQUAL "arm64")
      set(IOS_PLATFORM "iphoneos")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
      set(CMAKE_SYSTEM_PROCESSOR arm64)
    elseif(${IOS_ARCH} STREQUAL "arm64e")
      set(IOS_PLATFORM "iphoneos")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64e")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64e")
      set(CMAKE_SYSTEM_PROCESSOR arm64e)
    else()
      message(FATAL_ERROR "Unrecognized iOS architecture '${IOS_ARCH}'")
    endif()

    execute_process(COMMAND xcodebuild -version -sdk ${IOS_PLATFORM} ONLY_ACTIVE_ARCH=NO Path
      OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("-- CMAKE_OSX_SYSROOT       ${CMAKE_OSX_SYSROOT}")
    message("-- ARCHITECTURE:           ${CMAKE_SYSTEM_PROCESSOR}")
    message("-- PLATFORM:               ${IOS_PLATFORM}")
  else()
    if(${MAC_ARCH} STREQUAL "x86_64")
      set(CMAKE_SYSTEM_PROCESSOR x86_64)
    elseif(${MAC_ARCH} STREQUAL "arm64")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
      set(CMAKE_SYSTEM_PROCESSOR arm64)
      set(APPLE True)
    else()
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64 -arch arm64")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -arch arm64")
    endif()
    message("-- MAC_ARCH:               ${MAC_ARCH}")
  endif()
endif()

message("-- CMAKE_SYSTEM_INFO_FILE: ${CMAKE_SYSTEM_INFO_FILE}")
message("-- CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")
message("-- CMAKE_SYSTEM_VERSION:   ${CMAKE_SYSTEM_VERSION}")
message("-- CMAKE_BUILD_TYPE:       ${CMAKE_BUILD_TYPE}")
message("-- TARGET_ARCH:            ${TARGET_ARCH}")

include(tools/ParseOsRelease.cmake)

# No -pedantic -Wno-extra-semi -Wno-gnu-zero-variadic-macro-arguments
set(WARN_FLAGS "-Wall -Wextra -Wno-unused-parameter")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # Using GCC with -s and -Wl linker flags
  set(REL_FLAGS "-s -Wl,--gc-sections -Os ${WARN_FLAGS} -ffunction-sections -fdata-sections -fmerge-all-constants -ffast-math")
else()
  # Using clang - strip unsupported GCC options
  set(REL_FLAGS "-Os ${WARN_FLAGS} -ffunction-sections -fmerge-all-constants -ffast-math")
endif()

## Uncomment this to reduce the volume of note warnings on RPi4 w/gcc-8 Ref. https://gcc.gnu.org/ml/gcc/2017-05/msg00073.html
#if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
#  set(WARN_FLAGS "${WARN_FLAGS} -Wno-psabi"
#endif()

# Use libtcmalloc for Debug builds memory leaks detection
set(DBG_FLAGS "-ggdb -gdwarf-2 -O0  ${WARN_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  #TODO: -fno-rtti
  message("Building Release ...")
  set(CMAKE_C_FLAGS   "$ENV{CFLAGS} ${CMAKE_C_FLAGS} -std=c11 ${REL_FLAGS}")
  set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} ${CMAKE_CXX_FLAGS} -std=c++11 ${REL_FLAGS}")
else()
  set(USE_TCMALLOC 1)
  message("Building Debug ...")
  include(tools/FindTcmalloc.cmake)
  set(CMAKE_C_FLAGS   "$ENV{CFLAGS} ${CMAKE_C_FLAGS} -std=c11 ${DBG_FLAGS}")
  set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} ${CMAKE_CXX_FLAGS} -std=c++11 ${DBG_FLAGS}")
endif()

#Remove /Zi for Win32 debug compiler issue
if(MSVC)
  string( TOLOWER "${CMAKE_VS_PLATFORM_NAME}" PLATFORM_NAME_LOWER )
  if (PLATFORM_NAME_LOWER STREQUAL "win32")
    string(REGEX REPLACE "/Z[iI7]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REGEX REPLACE "/Z[iI7]" "" CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}")
  endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  # Prefer to generate position-independent code
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

include(tools/Utils.cmake)

if (NOT DEFAULT_PAL_IMPLEMENTATION)
# Linux, Mac OS X, MinGW, etc.
  set(DEFAULT_PAL_IMPLEMENTATION CPP11)
endif()

set(PAL_IMPLEMENTATION ${DEFAULT_PAL_IMPLEMENTATION})

message(STATUS "-- PAL implementation: ${PAL_IMPLEMENTATION}")
string(TOUPPER ${PAL_IMPLEMENTATION} PAL_IMPLEMENTATION_UPPER)
add_definitions(-DMATSDK_PAL_${PAL_IMPLEMENTATION_UPPER}=1)

option(GCC5_CXX11_ABI_WORKAROUND "Workaround: Use legacy C++11 ABI (for GCC 5 compatibility)" OFF)
if(GCC5_CXX11_ABI_WORKAROUND)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

if(PAL_IMPLEMENTATION STREQUAL "WIN32")
 add_definitions(-DZLIB_WINAPI)
endif()

################################################################################################
# Build prefix and version
################################################################################################

set(SDK_VERSION_PREFIX "EVT")
add_definitions("-DMATSDK_VERSION_PREFIX=\"${SDK_VERSION_PREFIX}\"")

set(MATSDK_API_VERSION "3.4")
string(TIMESTAMP DAYNUMBER "%j")
string(REGEX REPLACE "^00" "" DAYNUMBER ${DAYNUMBER})
string(REGEX REPLACE "^0"  "" DAYNUMBER ${DAYNUMBER})

if(DEFINED BUILD_VERSION AND NOT BUILD_VERSION STREQUAL "0.0.0.0")
  message(STATUS "Using externally set build version: ${BUILD_VERSION}")
  string(REPLACE "." "\\." MATSDK_API_VERSION_ESCAPED ${MATSDK_API_VERSION})
  if(NOT BUILD_VERSION MATCHES "^${MATSDK_API_VERSION_ESCAPED}\\.")
    message(WARNING "BUILD_VERSION (${BUILD_VERSION}) does not start with the current API version prefix (${MATSDK_API_VERSION}).")
  endif()
  set(MATSDK_BUILD_VERSION ${BUILD_VERSION})
else()
  set(MATSDK_BUILD_VERSION ${MATSDK_API_VERSION}.${DAYNUMBER}.0)
endif()

message(STATUS "-- SDK version: ${SDK_VERSION_PREFIX}-${MATSDK_BUILD_VERSION}")

################################################################################################
# HTTP stack section
################################################################################################

# Only use custom curl if compiling with CPP11 PAL
if (PAL_IMPLEMENTATION STREQUAL "CPP11")
  
  if(NOT BUILD_IOS)
    include(FindCURL)
    find_package(CURL REQUIRED)
    if (NOT CURL_FOUND)
      message (FATAL_ERROR "libcurl not found! Have you installed deps?")
    endif (NOT CURL_FOUND)
    include_directories(CURL_INCLUDE_DIRS)
    set(CMAKE_REQUIRED_LIBRARIES "${CURL_LIBRARIES}")
    list(APPEND LIBS "${CURL_LIBRARIES}")
  endif()
endif()

################################################################################################
# User options
################################################################################################
option(BUILD_HEADERS      "Build API headers"       YES)
option(BUILD_LIBRARY      "Build library"           YES)
option(BUILD_TEST_TOOL    "Build console test tool" YES)
option(BUILD_UNIT_TESTS   "Build unit tests"        YES)
option(BUILD_FUNC_TESTS   "Build functional tests"  YES)
option(BUILD_JNI_WRAPPER  "Build JNI wrapper"       NO)
option(BUILD_OBJC_WRAPPER "Build Obj-C wrapper"     YES)
option(BUILD_PACKAGE      "Build package"           YES)
option(BUILD_PRIVACYGUARD "Build Privacy Guard"     YES)
option(BUILD_CDS          "Build CDS - Common Diagnostic Stack"     YES)

# Enable Azure Monitor / Application Insights end-point support
option(BUILD_AZMON        "Build for Azure Monitor" YES)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  option(BUILD_APPLE_HTTP "Build Apple HTTP client" YES)
endif()

if(BUILD_UNIT_TESTS OR BUILD_FUNC_TESTS)
    message("Adding gtest")
    add_library(gtest STATIC IMPORTED GLOBAL)
    message("Adding gmock")
    add_library(gmock STATIC IMPORTED GLOBAL)
    
#    message("Building libraries! - Path: ${CMAKE_CURRENT_SOURCE_DIR} ")
#    message("Building gtest")
#    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/)
#    message("Building gmock")
#    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock/)
endif()

if(BUILD_APPLE_HTTP)
  add_definitions(-DAPPLE_HTTP=1)
endif()

# Bond Lite subdirectories
include_directories(bondlite/include)

include_directories(lib/pal)

#if(BUILD_UNIT_TESTS)
#  message("Adding bondlite tests")
#  enable_testing()
#  add_subdirectory(bondlite/tests)
#endif()

if(BUILD_HEADERS)
  add_subdirectory(lib/include)
endif()
include_directories(lib/include)

if(BUILD_LIBRARY)
  add_subdirectory(lib)
endif()

if(BUILD_UNIT_TESTS OR BUILD_FUNC_TESTS)
  message("Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()

################################################################################################
# Packaging
################################################################################################

if (BUILD_PACKAGE)
  if (${CMAKE_PACKAGE_TYPE} STREQUAL "deb")
    # FIXME: hardcode it for 64-bit Linux for now
    set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib/${CPACK_DEBIAN_ARCHITECTURE}-linux-gnu)
    include(tools/MakeDeb.cmake)
  endif()
  if (${CMAKE_PACKAGE_TYPE} STREQUAL "rpm")
    # TODO: [MG] - fix path
    set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
    include(tools/MakeRpm.cmake)
  endif()
  if (${CMAKE_PACKAGE_TYPE} STREQUAL "tgz")
    # TODO: [MG] - fix path... should we simply use /usr/local/lib without CPU?
    # TODO: [MG] - Windows path is not ideal -- C:/Program Files (x86)/MSTelemetry/* - what should we use instead?
    include(tools/MakeTgz.cmake)
  endif()
endif()
