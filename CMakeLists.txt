cmake_minimum_required(VERSION 3.15...3.31)

# Backward-compatible translation for legacy Apple build scripts. Standard
# CMAKE_OSX_* variables are the canonical interface and must be initialized
# before project() so CMake's compiler/platform detection sees them.
if(NOT DEFINED CMAKE_OSX_ARCHITECTURES)
  if(DEFINED IOS_ARCH AND NOT IOS_ARCH STREQUAL "")
    set(CMAKE_OSX_ARCHITECTURES "${IOS_ARCH}" CACHE STRING
      "Apple target architectures" FORCE)
  elseif(DEFINED MAC_ARCH AND NOT MAC_ARCH STREQUAL "")
    if(MAC_ARCH STREQUAL "universal")
      set(_matsdk_legacy_apple_architectures "arm64;x86_64")
    else()
      set(_matsdk_legacy_apple_architectures "${MAC_ARCH}")
    endif()
    set(CMAKE_OSX_ARCHITECTURES "${_matsdk_legacy_apple_architectures}" CACHE STRING
      "Apple target architectures" FORCE)
  endif()
endif()
if(NOT DEFINED CMAKE_OSX_SYSROOT AND DEFINED IOS_PLAT AND NOT IOS_PLAT STREQUAL "")
  set(CMAKE_OSX_SYSROOT "${IOS_PLAT}" CACHE STRING "Apple SDK sysroot" FORCE)
endif()
if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET
   AND DEFINED IOS_DEPLOYMENT_TARGET
   AND NOT IOS_DEPLOYMENT_TARGET STREQUAL "")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}" CACHE STRING
    "Apple deployment target" FORCE)
endif()

project(MSTelemetry LANGUAGES C CXX)
if(APPLE)
  set(MATSDK_BUILD_PLATFORM_APPLE TRUE)
else()
  set(MATSDK_BUILD_PLATFORM_APPLE FALSE)
endif()

################################################################################################
# Vcpkg dependency mode: detect early so it can guard platform-specific flag logic
################################################################################################
if(DEFINED VCPKG_TOOLCHAIN OR DEFINED VCPKG_TARGET_TRIPLET)
  option(MATSDK_USE_VCPKG_DEPS "Use vcpkg-provided dependencies via find_package()" ON)
else()
  option(MATSDK_USE_VCPKG_DEPS "Use vcpkg-provided dependencies via find_package()" OFF)
endif()
message(STATUS "MATSDK_USE_VCPKG_DEPS: ${MATSDK_USE_VCPKG_DEPS}")

set(MATSDK_SQLITE_TARGET "" CACHE STRING
  "Optional existing CMake target to use for SQLite in non-vcpkg builds, instead of find_package(SQLite3)")
set(MATSDK_ZLIB_TARGET "" CACHE STRING
  "Optional existing CMake target to use for zlib in non-vcpkg builds, instead of find_package(ZLIB)")

include(cmake/MatsdkOptions.cmake)

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

if(APPLE)
  if(MATSDK_BUILD_IOS
     OR CMAKE_SYSTEM_NAME STREQUAL "iOS"
     OR CMAKE_SYSTEM_NAME STREQUAL "visionOS")
    set(MATSDK_BUILD_IOS ON)
    set(TARGET_ARCH "APPLE")
    set(IOS TRUE)
  elseif(CMAKE_OSX_ARCHITECTURES)
    set(TARGET_ARCH "${CMAKE_OSX_ARCHITECTURES}")
  endif()
  message(STATUS "MATSDK_BUILD_IOS:       ${MATSDK_BUILD_IOS}")
  message(STATUS "CMAKE_OSX_ARCHITECTURES: ${CMAKE_OSX_ARCHITECTURES}")
  message(STATUS "CMAKE_OSX_SYSROOT:       ${CMAKE_OSX_SYSROOT}")
  message(STATUS "CMAKE_OSX_DEPLOYMENT_TARGET: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

message(STATUS "CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_VERSION:   ${CMAKE_SYSTEM_VERSION}")
message(STATUS "CMAKE_BUILD_TYPE:       ${CMAKE_BUILD_TYPE}")
message(STATUS "TARGET_ARCH:            ${TARGET_ARCH}")
message(STATUS "CMAKE_CXX_COMPILER_ID:  ${CMAKE_CXX_COMPILER_ID}")

include(tools/ParseOsRelease.cmake)

# SDK-owned compiler policy. This interface target is linked PRIVATE by SDK
# targets, so add_subdirectory()/FetchContent consumers and vendored dependency
# targets never inherit the SDK's warning-as-error or optimization policy.
add_library(matsdk_build_options INTERFACE)
if(MSVC)
  target_compile_options(matsdk_build_options INTERFACE
    /W4
    $<$<BOOL:${MATSDK_WARNINGS_AS_ERRORS}>:/WX>
    /Gy
    $<$<CXX_COMPILER_ID:MSVC>:/Gw>)
else()
  target_compile_options(matsdk_build_options INTERFACE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-but-set-variable
    $<$<BOOL:${MATSDK_WARNINGS_AS_ERRORS}>:-Werror>
    $<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:-Wno-unknown-warning-option>
    $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-unknown-warning-option>
    $<$<CONFIG:Debug>:-ggdb>
    $<$<CONFIG:Debug>:-gdwarf-2>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-fno-builtin-malloc>
    $<$<CONFIG:Debug>:-fno-builtin-calloc>
    $<$<CONFIG:Debug>:-fno-builtin-realloc>
    $<$<CONFIG:Debug>:-fno-builtin-free>
    $<$<NOT:$<CONFIG:Debug>>:-Os>
    $<$<NOT:$<CONFIG:Debug>>:-fmerge-all-constants>
    -ffunction-sections
    $<$<NOT:$<CXX_COMPILER_ID:AppleClang>>:-fdata-sections>)
  if(NOT WIN32)
    target_compile_options(matsdk_build_options INTERFACE
      -fvisibility=hidden
      $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>)
  endif()
endif()

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
# Embedding/dependency options
################################################################################################
set(MATSDK_CURL_TARGET "" CACHE STRING
  "Optional existing CMake target to use for libcurl in non-vcpkg builds, instead of find_package(CURL)")
set(MATSDK_CURL_PROVIDER "PACKAGE" CACHE STRING
  "How non-vcpkg builds resolve libcurl: PACKAGE (find_package) or FETCH (FetchContent static curl)")
set_property(CACHE MATSDK_CURL_PROVIDER PROPERTY STRINGS PACKAGE FETCH)
set(MATSDK_CURL_TLS_BACKEND "MBEDTLS" CACHE STRING
  "TLS backend for MATSDK_CURL_PROVIDER=FETCH: MBEDTLS or OPENSSL")
set_property(CACHE MATSDK_CURL_TLS_BACKEND PROPERTY STRINGS MBEDTLS OPENSSL)
set(MATSDK_CURL_URL "https://github.com/curl/curl/releases/download/curl-8_21_0/curl-8.21.0.tar.xz" CACHE STRING
  "URL for MATSDK_CURL_PROVIDER=FETCH")
set(MATSDK_CURL_SHA256 "aa1b66a70eace83dc624508745646c08ae561de512ab403adffb93ac87fc72e6" CACHE STRING
  "SHA256 for MATSDK_CURL_URL")
set(MATSDK_MBEDTLS_URL "https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.7/mbedtls-3.6.7.tar.bz2" CACHE STRING
  "URL for the mbedTLS dependency used by MATSDK_CURL_PROVIDER=FETCH and MATSDK_CURL_TLS_BACKEND=MBEDTLS")
set(MATSDK_MBEDTLS_SHA256 "a7e8bcbec0e6f761b4af24f25677626b35f762f68eef79c08677a363212d11f6" CACHE STRING
  "SHA256 for MATSDK_MBEDTLS_URL")
string(TOUPPER "${MATSDK_CURL_PROVIDER}" MATSDK_CURL_PROVIDER_UPPER)
if(NOT MATSDK_CURL_PROVIDER_UPPER STREQUAL "PACKAGE" AND NOT MATSDK_CURL_PROVIDER_UPPER STREQUAL "FETCH")
  message(FATAL_ERROR "MATSDK_CURL_PROVIDER must be PACKAGE or FETCH; got '${MATSDK_CURL_PROVIDER}'.")
endif()
string(TOUPPER "${MATSDK_CURL_TLS_BACKEND}" MATSDK_CURL_TLS_BACKEND_UPPER)
if(NOT MATSDK_CURL_TLS_BACKEND_UPPER STREQUAL "MBEDTLS" AND NOT MATSDK_CURL_TLS_BACKEND_UPPER STREQUAL "OPENSSL")
  message(FATAL_ERROR "MATSDK_CURL_TLS_BACKEND must be MBEDTLS or OPENSSL; got '${MATSDK_CURL_TLS_BACKEND}'.")
endif()
if(MATSDK_USE_VCPKG_DEPS AND MATSDK_CURL_PROVIDER_UPPER STREQUAL "FETCH")
  message(FATAL_ERROR
    "MATSDK_CURL_PROVIDER=FETCH is a non-vcpkg dependency mode. "
    "Use the vcpkg curl-openssl/curl-mbedtls feature instead.")
endif()
if(MATSDK_CURL_TARGET AND MATSDK_CURL_PROVIDER_UPPER STREQUAL "FETCH")
  message(FATAL_ERROR
    "MATSDK_CURL_TARGET and MATSDK_CURL_PROVIDER=FETCH are mutually exclusive.")
endif()
foreach(_matsdk_dependency_target_var MATSDK_CURL_TARGET MATSDK_SQLITE_TARGET MATSDK_ZLIB_TARGET)
  if(${_matsdk_dependency_target_var} AND NOT TARGET "${${_matsdk_dependency_target_var}}")
    message(FATAL_ERROR
      "${_matsdk_dependency_target_var} was set to '${${_matsdk_dependency_target_var}}', "
      "but no such CMake target exists. Define the target before adding "
      "cpp_client_telemetry, or leave ${_matsdk_dependency_target_var} empty to "
      "use the SDK's default dependency discovery.")
  endif()
endforeach()
if(MATSDK_BUILD_APPLE_HTTP)
  add_definitions(-DAPPLE_HTTP=1)
endif()

################################################################################################
# HTTP stack section
################################################################################################

# Only use custom curl if compiling with CPP11 PAL
set(MATSDK_NEEDS_CURL OFF)
if(PAL_IMPLEMENTATION STREQUAL "CPP11"
   AND NOT MATSDK_BUILD_IOS
   AND (NOT CMAKE_SYSTEM_NAME STREQUAL "Android" OR MATSDK_USE_VCPKG_DEPS)
   AND NOT MATSDK_BUILD_APPLE_HTTP)
  set(MATSDK_NEEDS_CURL ON)
  add_definitions(-DHAVE_MAT_CURL_HTTP_CLIENT)
  if(MATSDK_USE_VCPKG_DEPS)
    # The TLS backend (OpenSSL/mbedTLS) is selected by the vcpkg port's
    # curl-openssl (default) / curl-mbedtls features; the SDK just links libcurl.
    # Force CONFIG mode so the vcpkg-provided CURLConfig (which defines the
    # CURL::libcurl imported target) is used rather than the module FindCURL,
    # which on some CMake versions does not define that target.
    find_package(CURL CONFIG QUIET)
    if(NOT TARGET CURL::libcurl)
      message(FATAL_ERROR
        "libcurl was not found. The vcpkg port provides the curl HTTP client "
        "through the curl-openssl (default) or curl-mbedtls feature. Install "
        "cpp-client-telemetry with its default features, or, under the [core,...] "
        "form (which drops the default curl-openssl and system-sqlite features), "
        "re-select a curl backend and a SQLite backend together, e.g. "
        "[core,curl-openssl,system-sqlite] or [core,curl-mbedtls,minimal-sqlite].")
    endif()
    list(APPEND LIBS CURL::libcurl)
  else()
    if(MATSDK_CURL_TARGET)
      list(APPEND LIBS "${MATSDK_CURL_TARGET}")
    elseif(MATSDK_CURL_PROVIDER_UPPER STREQUAL "FETCH")
      include(cmake/MatsdkFetchCurl.cmake)
      matsdk_fetch_curl(_matsdk_curl_target)
      list(APPEND LIBS "${_matsdk_curl_target}")
    else()
      find_package(CURL REQUIRED)
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
endif()

################################################################################################
# Dependency resolution (vcpkg mode vs vendored)
################################################################################################
if(MATSDK_USE_VCPKG_DEPS)
  if(MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "SYSTEM" AND NOT MATSDK_SQLITE_TARGET)
    if(APPLE)
      find_package(SQLite3 REQUIRED)
    else()
      find_package(unofficial-sqlite3 CONFIG QUIET)
      if(NOT unofficial-sqlite3_FOUND)
        message(FATAL_ERROR
          "MATSDK_SQLITE_PROVIDER=SYSTEM requires the vcpkg sqlite3 package. "
          "Install the system-sqlite feature, choose "
          "MATSDK_SQLITE_PROVIDER=MINIMAL/VENDORED, or provide MATSDK_SQLITE_TARGET.")
      endif()
    endif()
  endif()
  if(MATSDK_ZLIB_PROVIDER_RESOLVED STREQUAL "SYSTEM" AND NOT MATSDK_ZLIB_TARGET)
    find_package(ZLIB REQUIRED)
  endif()
  find_package(nlohmann_json CONFIG REQUIRED)
  message(STATUS
    "Using vcpkg dependency mode with SQLite=${MATSDK_SQLITE_PROVIDER_RESOLVED}, "
    "zlib=${MATSDK_ZLIB_PROVIDER_RESOLVED}")
else()
  # nlohmann JSON remains header-only and vendored for source embedding.
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  message(STATUS
    "Using source dependency mode with SQLite=${MATSDK_SQLITE_PROVIDER_RESOLVED}, "
    "zlib=${MATSDK_ZLIB_PROVIDER_RESOLVED}, vendored nlohmann-json")
endif()

if(MATSDK_BUILD_UNIT_TESTS OR MATSDK_BUILD_FUNC_TESTS)
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
if(MATSDK_BUILD_HEADERS)
  add_subdirectory(lib/include)
endif()
include_directories(lib/include)

if(MATSDK_BUILD_LIBRARY)
  add_subdirectory(lib)
endif()

if(MATSDK_BUILD_UNIT_TESTS OR MATSDK_BUILD_FUNC_TESTS)
  message(STATUS "Building tests")
  enable_testing()
  add_subdirectory(tests)
endif()

################################################################################################
# Packaging
################################################################################################

if(MATSDK_BUILD_PACKAGE)
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
