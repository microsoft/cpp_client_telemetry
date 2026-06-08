cmake_minimum_required(VERSION 3.5)
project(MSTelemetry LANGUAGES C CXX)

################################################################################################
# Vcpkg dependency mode: detect early so it can guard platform-specific flag logic
################################################################################################
if(DEFINED VCPKG_TOOLCHAIN OR DEFINED VCPKG_TARGET_TRIPLET)
  option(MATSDK_USE_VCPKG_DEPS "Use vcpkg-provided dependencies via find_package()" ON)
else()
  option(MATSDK_USE_VCPKG_DEPS "Use vcpkg-provided dependencies via find_package()" OFF)
endif()
message(STATUS "MATSDK_USE_VCPKG_DEPS: ${MATSDK_USE_VCPKG_DEPS}")

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
# Initialize platform options before conditional blocks (needed for config templates)
if(NOT DEFINED BUILD_IOS)
  set(BUILD_IOS OFF)
endif()
if(NOT APPLE AND NOT DEFINED BUILD_APPLE_HTTP)
  set(BUILD_APPLE_HTTP OFF)
endif()

if(APPLE)
  message(STATUS "BUILD_IOS:              ${BUILD_IOS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")

  # iOS build options
  option(BUILD_IOS           "Build for iOS"               NO)
  option(FORCE_RESET_OSX_DEPLOYMENT_TARGET "Clear the OSX Deployment Target Set" YES)
  if (DEFINED FORCE_RESET_DEPLOYMENT_TARGET)
    set(FORCE_RESET_OSX_DEPLOYMENT_TARGET ${FORCE_RESET_DEPLOYMENT_TARGET})
  endif()

  # When building via vcpkg, the toolchain file handles architecture, sysroot,
  # deployment target, and platform flags. Skip manual flag configuration.
  if(NOT MATSDK_USE_VCPKG_DEPS)
    if(BUILD_IOS)
      set(TARGET_ARCH "APPLE")
      set(IOS True)
      set(APPLE True)

      if(FORCE_RESET_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "Force unset of the deployment target for iOS" FORCE)
        if (${IOS_PLAT} STREQUAL "iphonesimulator")
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mios-simulator-version-min=${IOS_DEPLOYMENT_TARGET}")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mios-simulator-version-min=${IOS_DEPLOYMENT_TARGET}")
        else()
          set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -miphoneos-version-min=${IOS_DEPLOYMENT_TARGET}")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -miphoneos-version-min=${IOS_DEPLOYMENT_TARGET}")
        endif()
      endif()

      if((${IOS_PLAT} STREQUAL "iphoneos") OR (${IOS_PLAT} STREQUAL "iphonesimulator") OR (${IOS_PLAT} STREQUAL "xros") OR (${IOS_PLAT} STREQUAL "xrsimulator"))
        set(IOS_PLATFORM "${IOS_PLAT}")
      else()
        message(FATAL_ERROR "Unrecognized iOS platform '${IOS_PLAT}'")
      endif()

      if(${IOS_ARCH} STREQUAL "x86_64")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64")
        set(CMAKE_SYSTEM_PROCESSOR x86_64)
      elseif(${IOS_ARCH} STREQUAL "arm64")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
        set(CMAKE_SYSTEM_PROCESSOR arm64)
      elseif(${IOS_ARCH} STREQUAL "arm64e")
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
      message(STATUS "CMAKE_OSX_SYSROOT       ${CMAKE_OSX_SYSROOT}")
      message(STATUS "ARCHITECTURE:           ${CMAKE_SYSTEM_PROCESSOR}")
      message(STATUS "PLATFORM:               ${IOS_PLATFORM}")
    else()
      if("${MAC_ARCH}" STREQUAL "x86_64")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64")
        set(CMAKE_SYSTEM_PROCESSOR x86_64)
        set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
        set(CMAKE_OSX_ARCHITECTURES ${MAC_ARCH})
        set(APPLE True)
      elseif("${MAC_ARCH}" STREQUAL "arm64")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch arm64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
        set(CMAKE_SYSTEM_PROCESSOR arm64)
        set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
        set(CMAKE_OSX_ARCHITECTURES ${MAC_ARCH})
        set(APPLE True)
      else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64 -arch arm64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -arch arm64")
      endif()
      message(STATUS "MAC_ARCH:               ${MAC_ARCH}")
    endif()
  else()
    # vcpkg mode: just set internal flags from what the toolchain provides
    if(BUILD_IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
      set(BUILD_IOS ON)
      set(TARGET_ARCH "APPLE")
      set(IOS True)
    endif()
    message(STATUS "vcpkg toolchain managing architecture and platform flags")
  endif()
endif()

message(STATUS "CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_VERSION:   ${CMAKE_SYSTEM_VERSION}")
message(STATUS "CMAKE_BUILD_TYPE:       ${CMAKE_BUILD_TYPE}")
message(STATUS "TARGET_ARCH:            ${TARGET_ARCH}")
message(STATUS "CMAKE_CXX_COMPILER_ID:  ${CMAKE_CXX_COMPILER_ID}")

include(tools/ParseOsRelease.cmake)

# When building via vcpkg, let the toolchain manage compiler flags.
# Only apply project-specific flags for non-vcpkg (legacy) builds.
if(NOT MATSDK_USE_VCPKG_DEPS)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(WARN_FLAGS "/W4 /WX")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # -Wno-unknown-warning-option is Clang-only, omitted here
  set(WARN_FLAGS "-Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-but-set-variable")
  # -Wno-reorder is C++-only; added to CXX_FLAGS below (suppresses member-init-order warnings in submodule code)
  set(CXX_EXTRA_WARN_FLAGS "-Wno-reorder")
else()
  # Clang / AppleClang
  set(WARN_FLAGS "-Wall -Werror -Wextra -Wno-unused-parameter -Wno-unknown-warning-option -Wno-unused-but-set-variable")
  # -Wno-reorder is C++-only; added to CXX_FLAGS below (suppresses member-init-order warnings in submodule code)
  set(CXX_EXTRA_WARN_FLAGS "-Wno-reorder")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # Using GCC with -s and -Wl linker flags
  set(REL_FLAGS "-s -Wl,--gc-sections -Os ${WARN_FLAGS} -ffunction-sections -fdata-sections -fmerge-all-constants")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(REL_FLAGS "${WARN_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  # AppleClang does not support -ffunction-sections and -fdata-sections with the -fembed-bitcode and -fembed-bitcode-marker
  set(REL_FLAGS "-Os ${WARN_FLAGS} -fmerge-all-constants")
else()
  # Using clang - strip unsupported GCC options
  set(REL_FLAGS "-Os ${WARN_FLAGS} -ffunction-sections -fmerge-all-constants")
endif()

## Uncomment this to reduce the volume of note warnings on RPi4 w/gcc-8 Ref. https://gcc.gnu.org/ml/gcc/2017-05/msg00073.html
#if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
#  set(WARN_FLAGS "${WARN_FLAGS} -Wno-psabi"
#endif()

# Use libtcmalloc for Debug builds memory leaks detection
set(DBG_FLAGS "-ggdb -gdwarf-2 -O0  ${WARN_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  #TODO: -fno-rtti
  message(STATUS "Building Release ...")
  set(CMAKE_C_FLAGS   "$ENV{CFLAGS} ${CMAKE_C_FLAGS} -std=c11 ${REL_FLAGS}")
  set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} ${CMAKE_CXX_FLAGS} -std=c++11 ${REL_FLAGS} ${CXX_EXTRA_WARN_FLAGS}")
else()
  set(USE_TCMALLOC 1)
  message(STATUS "Building Debug ...")
  include(tools/FindTcmalloc.cmake)
  set(CMAKE_C_FLAGS   "$ENV{CFLAGS} ${CMAKE_C_FLAGS} -std=c11 ${DBG_FLAGS}")
  set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} ${CMAKE_CXX_FLAGS} -std=c++11 ${DBG_FLAGS} ${CXX_EXTRA_WARN_FLAGS}")
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

endif() # NOT MATSDK_USE_VCPKG_DEPS (compiler flags)

include(tools/Utils.cmake)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (NOT DEFAULT_PAL_IMPLEMENTATION)
if(MSVC)
  set(DEFAULT_PAL_IMPLEMENTATION WIN32)
else()
# Linux, Mac OS X, MinGW, etc.
  set(DEFAULT_PAL_IMPLEMENTATION CPP11)
endif()
endif()

set(PAL_IMPLEMENTATION ${DEFAULT_PAL_IMPLEMENTATION})

message(STATUS "PAL implementation: ${PAL_IMPLEMENTATION}")
string(TOUPPER ${PAL_IMPLEMENTATION} PAL_IMPLEMENTATION_UPPER)
add_definitions(-DMATSDK_PAL_${PAL_IMPLEMENTATION_UPPER}=1)

option(GCC5_CXX11_ABI_WORKAROUND "Workaround: Use legacy C++11 ABI (for GCC 5 compatibility)" OFF)
if(GCC5_CXX11_ABI_WORKAROUND)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

option(USE_ONEDS_BOUNDCHECK_METHODS "Use bound check methods for C99 functions" OFF)
if (USE_ONEDS_BOUNDCHECK_METHODS)
  add_definitions(-DHAVE_ONEDS_BOUNDCHECK_METHODS)
endif()

option(USE_ONEDS_SECURE_MEM_FUNCTIONS "Use secure memory functions for sqlite" OFF)
if(USE_ONEDS_SECURE_MEM_FUNCTIONS)
  add_definitions(-DUSE_ONEDS_SECURE_MEM_FUNCTIONS)
endif()

if(PAL_IMPLEMENTATION STREQUAL "WIN32" AND NOT MATSDK_USE_VCPKG_DEPS)
  add_definitions(-DZLIB_WINAPI)
endif()

add_definitions(-DNOMINMAX)

################################################################################################
# Build prefix and version
################################################################################################

set(SDK_VERSION_PREFIX "EVT")
add_definitions("-DMATSDK_VERSION_PREFIX=\"${SDK_VERSION_PREFIX}\"")

set(MATSDK_API_VERSION "3.10")
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

message(STATUS "SDK version: ${SDK_VERSION_PREFIX}-${MATSDK_BUILD_VERSION}")

################################################################################################
# User options (must be before HTTP stack section for BUILD_APPLE_HTTP)
################################################################################################
option(BUILD_HEADERS      "Build API headers"       YES)
option(BUILD_LIBRARY      "Build library"           YES)
option(BUILD_TEST_TOOL    "Build console test tool" YES)
option(BUILD_UNIT_TESTS   "Build unit tests"        YES)
option(BUILD_FUNC_TESTS   "Build functional tests"  YES)
option(BUILD_JNI_WRAPPER  "Build JNI wrapper"       NO)
option(BUILD_OBJC_WRAPPER "Build Obj-C wrapper"     YES)
option(BUILD_SWIFT_WRAPPER "Build Swift Wrappers"   YES)
option(BUILD_PACKAGE      "Build package"           YES)
option(BUILD_PRIVACYGUARD "Build Privacy Guard"     YES)
option(BUILD_CDS          "Build CDS - Common Diagnostic Stack"     YES)
option(BUILD_LIVEEVENTINSPECTOR   "Build Live Event Inspector"      YES)
option(BUILD_SIGNALS      "Build Signals"           YES)
option(BUILD_SANITIZER    "Build Sanitizer"         YES)
option(LINK_STATIC_DEPENDS "Link dependencies for static build"     YES)

# Enable Azure Monitor / Application Insights end-point support
option(BUILD_AZMON        "Build for Azure Monitor" YES)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  option(BUILD_APPLE_HTTP "Build Apple HTTP client" YES)
endif()

if(BUILD_APPLE_HTTP)
  add_definitions(-DAPPLE_HTTP=1)
endif()

################################################################################################
# HTTP stack section
################################################################################################

# Only use custom curl if compiling with CPP11 PAL
set(MATSDK_NEEDS_CURL OFF)
if(PAL_IMPLEMENTATION STREQUAL "CPP11"
   AND NOT BUILD_IOS
   AND (NOT CMAKE_SYSTEM_NAME STREQUAL "Android" OR MATSDK_USE_VCPKG_DEPS)
   AND NOT BUILD_APPLE_HTTP)
  set(MATSDK_NEEDS_CURL ON)
  add_definitions(-DHAVE_MAT_CURL_HTTP_CLIENT)
  find_package(CURL REQUIRED)
  if(MATSDK_USE_VCPKG_DEPS)
    list(APPEND LIBS CURL::libcurl)
  else()
    # Prefer the imported target, which carries curl's include dirs and link
    # flags. Fall back to the find-module variables on CMake < 3.12, where
    # find_package(CURL) does not define CURL::libcurl.
    if(TARGET CURL::libcurl)
      list(APPEND LIBS CURL::libcurl)
    else()
      include_directories(${CURL_INCLUDE_DIRS})
      list(APPEND LIBS "${CURL_LIBRARIES}")
    endif()
  endif()
endif()

################################################################################################
# Dependency resolution (vcpkg mode vs vendored)
################################################################################################
if(MATSDK_USE_VCPKG_DEPS)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
  message(STATUS "Using vcpkg-provided sqlite3, zlib, nlohmann-json")
else()
  # Include repo root to allow includes of vendored sqlite, zlib, and nlohmann
  include_directories(${CMAKE_SOURCE_DIR})
  message(STATUS "Using vendored sqlite3, zlib, nlohmann-json")
endif()

if(BUILD_UNIT_TESTS OR BUILD_FUNC_TESTS)
    message(STATUS "Adding gtest")
    add_library(gtest STATIC IMPORTED GLOBAL)
    message(STATUS "Adding gmock")
    add_library(gmock STATIC IMPORTED GLOBAL)
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
  message(STATUS "Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()

################################################################################################
# Packaging
################################################################################################

if (BUILD_PACKAGE)
  if ("${CMAKE_PACKAGE_TYPE}" STREQUAL "deb")
    include(tools/MakeDeb.cmake)
  endif()
  if ("${CMAKE_PACKAGE_TYPE}" STREQUAL "rpm")
    include(tools/MakeRpm.cmake)
  endif()
  if ("${CMAKE_PACKAGE_TYPE}" STREQUAL "tgz")
    # TODO: [MG] - fix path... should we simply use /usr/local/lib without CPU?
    # TODO: [MG] - Windows path is not ideal -- C:/Program Files (x86)/MSTelemetry/* - what should we use instead?
    include(tools/MakeTgz.cmake)
  endif()
endif()
