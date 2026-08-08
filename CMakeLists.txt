cmake_minimum_required(VERSION 3.15...3.31)

project(MSTelemetry LANGUAGES C CXX)
if(APPLE)
  set(MATSDK_BUILD_PLATFORM_APPLE TRUE)
else()
  set(MATSDK_BUILD_PLATFORM_APPLE FALSE)
endif()

################################################################################################
# Package-manager detection (internal; dependency selection is target/provider based)
################################################################################################
set(MATSDK_USING_VCPKG OFF)
if(DEFINED VCPKG_TOOLCHAIN OR DEFINED VCPKG_TARGET_TRIPLET)
  set(MATSDK_USING_VCPKG ON)
endif()
message(STATUS "MATSDK_USING_VCPKG: ${MATSDK_USING_VCPKG}")

include(cmake/MatsdkOptions.cmake)
include(cmake/MatsdkDependencyTargets.cmake)
if(APPLE)
  include(cmake/MatsdkAppleSystemDeps.cmake)
endif()

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
  if(MATSDK_PLATFORM_IOS)
    set(TARGET_ARCH "APPLE")
    set(IOS TRUE)
  elseif(CMAKE_OSX_ARCHITECTURES)
    set(TARGET_ARCH "${CMAKE_OSX_ARCHITECTURES}")
  endif()
  message(STATUS "MATSDK_PLATFORM_IOS:    ${MATSDK_PLATFORM_IOS}")
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
    $<$<CXX_COMPILER_ID:GNU,Clang>:-fdata-sections>)
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
add_library(matsdk_internal_config INTERFACE)
target_compile_definitions(matsdk_internal_config INTERFACE
  MATSDK_PAL_${PAL_IMPLEMENTATION_UPPER}=1
  NOMINMAX)

option(GCC5_CXX11_ABI_WORKAROUND "Workaround: Use legacy C++11 ABI (for GCC 5 compatibility)" OFF)
if(GCC5_CXX11_ABI_WORKAROUND)
  target_compile_definitions(matsdk_internal_config INTERFACE
    _GLIBCXX_USE_CXX11_ABI=0)
endif()

option(USE_ONEDS_BOUNDCHECK_METHODS "Use bound check methods for C99 functions" OFF)
if (USE_ONEDS_BOUNDCHECK_METHODS)
  target_compile_definitions(matsdk_internal_config INTERFACE
    HAVE_ONEDS_BOUNDCHECK_METHODS)
endif()

option(USE_ONEDS_SECURE_MEM_FUNCTIONS "Use secure memory functions for sqlite" OFF)
################################################################################################
# Build prefix and version
################################################################################################

set(SDK_VERSION_PREFIX "EVT")
target_compile_definitions(matsdk_internal_config INTERFACE
  "MATSDK_VERSION_PREFIX=\"${SDK_VERSION_PREFIX}\"")
if(MATSDK_ANDROID_USE_ROOM)
  target_compile_definitions(matsdk_internal_config INTERFACE USE_ROOM)
endif()
if(MATSDK_ENABLE_CAPI_HTTP_CLIENT)
  target_compile_definitions(matsdk_internal_config INTERFACE
    ENABLE_CAPI_HTTP_CLIENT)
endif()

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
set(MATSDK_CURL_PROVIDER "SYSTEM" CACHE STRING
  "How builds resolve libcurl: SYSTEM (canonical target/find_package) or FETCH")
set_property(CACHE MATSDK_CURL_PROVIDER PROPERTY STRINGS SYSTEM FETCH)
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
if(NOT MATSDK_CURL_PROVIDER_UPPER STREQUAL "SYSTEM" AND NOT MATSDK_CURL_PROVIDER_UPPER STREQUAL "FETCH")
  message(FATAL_ERROR "MATSDK_CURL_PROVIDER must be SYSTEM or FETCH; got '${MATSDK_CURL_PROVIDER}'.")
endif()
string(TOUPPER "${MATSDK_CURL_TLS_BACKEND}" MATSDK_CURL_TLS_BACKEND_UPPER)
if(MATSDK_CURL_PROVIDER_UPPER STREQUAL "FETCH"
   AND NOT MATSDK_CURL_TLS_BACKEND_UPPER STREQUAL "MBEDTLS"
   AND NOT MATSDK_CURL_TLS_BACKEND_UPPER STREQUAL "OPENSSL")
  message(FATAL_ERROR "MATSDK_CURL_TLS_BACKEND must be MBEDTLS or OPENSSL; got '${MATSDK_CURL_TLS_BACKEND}'.")
endif()
if(MATSDK_USING_VCPKG AND MATSDK_CURL_PROVIDER_UPPER STREQUAL "FETCH")
  message(FATAL_ERROR
    "MATSDK_CURL_PROVIDER=FETCH is a non-vcpkg dependency mode. "
    "Use the vcpkg curl-openssl/curl-mbedtls feature instead.")
endif()
if(MATSDK_BUILD_APPLE_HTTP)
  target_compile_definitions(matsdk_internal_config INTERFACE APPLE_HTTP=1)
endif()

################################################################################################
# HTTP stack section
################################################################################################

# Only use custom curl if compiling with CPP11 PAL
set(MATSDK_NEEDS_CURL OFF)
set(MATSDK_CURL_FETCHED OFF)
set(MATSDK_CURL_LINK_TARGET "")
if(PAL_IMPLEMENTATION STREQUAL "CPP11"
   AND NOT MATSDK_PLATFORM_IOS
   AND (NOT CMAKE_SYSTEM_NAME STREQUAL "Android" OR MATSDK_ANDROID_USES_CURL)
   AND NOT MATSDK_BUILD_APPLE_HTTP)
  set(MATSDK_NEEDS_CURL ON)
  target_compile_definitions(matsdk_internal_config INTERFACE
    HAVE_MAT_CURL_HTTP_CLIENT)
  if(TARGET CURL::libcurl)
    set(MATSDK_CURL_LINK_TARGET CURL::libcurl)
  elseif(MATSDK_CURL_PROVIDER_UPPER STREQUAL "FETCH")
    include(cmake/MatsdkFetchCurl.cmake)
    matsdk_fetch_curl(_matsdk_curl_target)
    set(MATSDK_CURL_LINK_TARGET "${_matsdk_curl_target}")
    set(MATSDK_CURL_FETCHED ON)
  else()
    find_package(CURL REQUIRED)
    if(NOT TARGET CURL::libcurl)
      message(FATAL_ERROR
        "find_package(CURL) did not create the required CURL::libcurl target.")
    endif()
    set(MATSDK_CURL_LINK_TARGET CURL::libcurl)
  endif()
endif()

################################################################################################
# Canonical dependency targets
################################################################################################
if(TARGET SQLite3::SQLite3 AND NOT TARGET SQLite::SQLite3)
  add_library(SQLite::SQLite3 ALIAS SQLite3::SQLite3)
endif()
if(MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "SYSTEM" AND NOT TARGET SQLite::SQLite3)
  if(APPLE AND NOT MATSDK_USING_VCPKG)
    matsdk_add_apple_system_library(SQLite::SQLite3 sqlite3)
  else()
    find_package(SQLite3 QUIET)
    if(NOT TARGET SQLite::SQLite3 AND MATSDK_USING_VCPKG)
      find_package(unofficial-sqlite3 CONFIG REQUIRED)
      matsdk_add_interface_dependency(
        SQLite::SQLite3 unofficial::sqlite3::sqlite3)
    endif()
    if(NOT TARGET SQLite::SQLite3)
      message(FATAL_ERROR
        "MATSDK_SQLITE_PROVIDER=SYSTEM requires SQLite::SQLite3. Install SQLite, "
        "define the canonical target before adding 1DS, or choose MINIMAL/VENDORED.")
    endif()
  endif()
endif()
if(TARGET SQLite::SQLite3 AND NOT TARGET SQLite3::SQLite3)
  get_target_property(_matsdk_sqlite_target SQLite::SQLite3 ALIASED_TARGET)
  if(_matsdk_sqlite_target)
    add_library(SQLite3::SQLite3 ALIAS ${_matsdk_sqlite_target})
  else()
    add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
  endif()
endif()

if(MATSDK_ZLIB_PROVIDER_RESOLVED STREQUAL "SYSTEM" AND NOT TARGET ZLIB::ZLIB)
  if(APPLE AND NOT MATSDK_USING_VCPKG)
    matsdk_add_apple_system_library(ZLIB::ZLIB z)
  else()
    find_package(ZLIB REQUIRED)
  endif()
endif()

set(MATSDK_USES_NLOHMANN_TARGET OFF)
if(TARGET nlohmann_json::nlohmann_json)
  set(MATSDK_USES_NLOHMANN_TARGET ON)
elseif(MATSDK_USING_VCPKG)
  find_package(nlohmann_json CONFIG REQUIRED)
  set(MATSDK_USES_NLOHMANN_TARGET ON)
else()
  # nlohmann JSON remains header-only and vendored for source embedding.
  set(MATSDK_USES_VENDORED_NLOHMANN ON)
endif()
message(STATUS
  "Dependencies: SQLite=${MATSDK_SQLITE_PROVIDER_RESOLVED}, "
  "zlib=${MATSDK_ZLIB_PROVIDER_RESOLVED}, "
  "nlohmann-target=${MATSDK_USES_NLOHMANN_TARGET}")

if(MATSDK_BUILD_UNIT_TESTS OR MATSDK_BUILD_FUNC_TESTS)
  if(NOT TARGET gtest OR NOT TARGET gmock)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/CMakeLists.txt")
      set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
      set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
      if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
      endif()
      set(_matsdk_saved_build_shared_libs "${BUILD_SHARED_LIBS}")
      set(BUILD_SHARED_LIBS OFF)
      add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)
      set(BUILD_SHARED_LIBS "${_matsdk_saved_build_shared_libs}")
      # Checked-in iOS test projects consume these archive paths directly.
      set(_matsdk_gtest_archive_dir
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest/build/lib")
      foreach(_matsdk_gtest_target IN ITEMS gtest gmock)
        set_target_properties(${_matsdk_gtest_target} PROPERTIES
          ARCHIVE_OUTPUT_DIRECTORY "${_matsdk_gtest_archive_dir}")
        foreach(_matsdk_gtest_config IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
          set_target_properties(${_matsdk_gtest_target} PROPERTIES
            "ARCHIVE_OUTPUT_DIRECTORY_${_matsdk_gtest_config}"
              "${_matsdk_gtest_archive_dir}")
        endforeach()
      endforeach()
    else()
      message(FATAL_ERROR
        "Tests require the third_party/googletest submodule. "
        "Run git submodule update --init third_party/googletest.")
    endif()
  endif()
endif()

#if(BUILD_UNIT_TESTS)
#  message("Adding bondlite tests")
#  enable_testing()
#  add_subdirectory(bondlite/tests)
#endif()
if(MATSDK_BUILD_HEADERS)
  add_subdirectory(lib/include)
endif()

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

if(DEFINED CMAKE_PACKAGE_TYPE AND NOT DEFINED CPACK_GENERATOR)
  string(TOUPPER "${CMAKE_PACKAGE_TYPE}" CPACK_GENERATOR)
  message(DEPRECATION
    "CMAKE_PACKAGE_TYPE is deprecated; use standard CPACK_GENERATOR.")
endif()
if(MATSDK_BUILD_PACKAGE)
  if("DEB" IN_LIST CPACK_GENERATOR)
    include(tools/MakeDeb.cmake)
  endif()
  if("RPM" IN_LIST CPACK_GENERATOR)
    include(tools/MakeRpm.cmake)
  endif()
  if("TGZ" IN_LIST CPACK_GENERATOR)
    # TODO: [MG] - fix path... should we simply use /usr/local/lib without CPU?
    # TODO: [MG] - Windows path is not ideal -- C:/Program Files (x86)/MSTelemetry/* - what should we use instead?
    include(tools/MakeTgz.cmake)
  endif()
endif()
