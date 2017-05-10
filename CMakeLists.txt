cmake_minimum_required(VERSION 3.1.0)
project(AriaSDK)

include(tools/Utils.cmake)


# Stand-alone or Skype (dev_buildtools etc.) build

if(DEFINED QB_CONFIGID)
  set(SKYPE YES)
  set(DEFAULT_PAL_IMPLEMENTATION Skype)
  set(BUILD_VERSION ${QB_BUILDVERSION})
  set(SYSROOT ${CFW_SYSROOT})
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/dev_buildtools/cmake/skylib_components")
  set_property(GLOBAL PROPERTY CFW_WARNERR ON)
else()
  set(SKYPE NO)
  set(DEFAULT_PAL_IMPLEMENTATION Win32)
  set(SYSROOT ${CMAKE_SOURCE_DIR}/sysroot)
endif()


# User options

option(BUILD_HEADERS      "Build API headers"       YES)
option(BUILD_LIBRARY      "Build library"           YES)
option(BUILD_TEST_TOOL    "Build console test tool" YES)
option(BUILD_UNIT_TESTS   "Build unit tests"        NO)
option(BUILD_FUNC_TESTS   "Build functional tests"  NO)

option(BUILD_JNI_WRAPPER  "Build JNI wrapper"       NO)
if(QB_CONFIGID MATCHES "^android-")
  set(BUILD_JNI_WRAPPER YES)
endif()

set(PAL_IMPLEMENTATION ${DEFAULT_PAL_IMPLEMENTATION} CACHE STRING "Platform abstraction layer implementation")
set_property(CACHE PAL_IMPLEMENTATION PROPERTY STRINGS "Skype" "Win32")
message(STATUS "PAL implementation: ${PAL_IMPLEMENTATION}")
string(TOUPPER ${PAL_IMPLEMENTATION} PAL_IMPLEMENTATION_UPPER)
add_definitions(-DARIASDK_PAL_${PAL_IMPLEMENTATION_UPPER}=1)

option(GCC5_CXX11_ABI_WORKAROUND "Workaround: Use legacy C++11 ABI (for GCC 5 compatibility with Skype dependencies)" OFF)
if(GCC5_CXX11_ABI_WORKAROUND)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
endif()

option(ARIASDK_UTC_ENABLED "Enabled event forwarding to Windows UTC (experimental)" OFF)
if(ARIASDK_UTC_ENABLED)
  add_definitions(-DARIASDK_UTC_ENABLED=1)
endif()


# Build version

set(SDK_VERSION_PREFIX "SCT2" CACHE STRING "SDK version prefix")
message(STATUS "SDK version prefix: ${SDK_VERSION_PREFIX}")
add_definitions("-DARIASDK_VERSION_PREFIX=\"${SDK_VERSION_PREFIX}\"")

set(ARIASDK_API_VERSION "5.0")
if(DEFINED BUILD_VERSION AND NOT BUILD_VERSION STREQUAL "0.0.0.0")
  message(STATUS "Using externally set build version: ${BUILD_VERSION}")
  string(REPLACE "." "\\." ARIASDK_API_VERSION_ESCAPED ${ARIASDK_API_VERSION})
  if(NOT BUILD_VERSION MATCHES "^${ARIASDK_API_VERSION_ESCAPED}\\.")
    message(WARNING "BUILD_VERSION (${BUILD_VERSION}) does not start with the current API version prefix (${ARIASDK_API_VERSION}).")
  endif()
  set(ARIASDK_BUILD_VERSION ${BUILD_VERSION})
else()
  message(STATUS "Using dummy build version (based on current API version ${ARIASDK_API_VERSION}): ${ARIASDK_API_VERSION}.0.0")
  set(ARIASDK_BUILD_VERSION ${ARIASDK_API_VERSION}.0.0)
endif()


# External depedencies

if(SKYPE)
  set_property(GLOBAL PROPERTY CFW_ENABLE_UNIFIED_LOGGING ON)
  if(CMAKE_BUILD_TYPE STREQUAL "release")
    set(LOGMAP_ENABLED yes)
  endif()

  if(PAL_IMPLEMENTATION STREQUAL "Skype")
    # Workaround for Resource Manager circular dependency
    add_library(clienttelemetry_lib INTERFACE)
  endif()

  include(roottools)
  include(httpstack)
  include(resourcemanager)
  include(ecsclient)
  include(json)
  include(zlib)
  include(sqlite)

  if(PAL_IMPLEMENTATION STREQUAL "Win32")
    # Unfortunately, ALIAS is not supported on IMPORTED targets.
    add_library(sqlite3 STATIC IMPORTED GLOBAL)
    get_target_property(SQLITE_IMPORTED_LOCATION_DEBUG    sqlite_lib IMPORTED_LOCATION_DEBUG)
    get_target_property(SQLITE_IMPORTED_LOCATION_RELEASE  sqlite_lib IMPORTED_LOCATION_RELEASE)
    get_target_property(SQLITE_IMPORTED_LOCATION_INTERNAL sqlite_lib IMPORTED_LOCATION_INTERNAL)
    set_target_properties(sqlite3 PROPERTIES
      IMPORTED_LOCATION_DEBUG    ${SQLITE_IMPORTED_LOCATION_DEBUG}
      IMPORTED_LOCATION_RELEASE  ${SQLITE_IMPORTED_LOCATION_RELEASE}
      IMPORTED_LOCATION_INTERNAL ${SQLITE_IMPORTED_LOCATION_INTERNAL}
    )
  endif()

  if(BUILD_UNIT_TESTS OR BUILD_FUNC_TESTS)
    include(gtest)
    include(gmock)
    set_target_properties(gtest PROPERTIES INTERFACE_COMPILE_DEFINITIONS
      GTEST_HAS_RTTI=1;GTEST_HAS_PTHREAD=0;GTEST_HAS_EXCEPTIONS=0;GTEST_HAS_TR1_TUPLE=1;_VARIADIC_MAX=10
    )
    # Workaround: At least for linux-x64_gcc, gtest.a is compiled without GTEST_HAS_PTHREAD=0
    if(QB_CONFIGID MATCHES "^linux-*")
      set_property(TARGET gtest APPEND PROPERTY INTERFACE_LINK_LIBRARIES -lpthread)
    endif()
  endif()
else()
  add_library(z STATIC IMPORTED GLOBAL)
  set_target_properties(z PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${SYSROOT}/include
    IMPORTED_LOCATION             ${SYSROOT}/lib/z.lib
  )

  add_library(sqlite3 STATIC IMPORTED GLOBAL)
  set_target_properties(sqlite3 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${SYSROOT}/include/sqlite
    IMPORTED_LOCATION             ${SYSROOT}/lib/debug/sqlite.lib
  )

  if(BUILD_UNIT_TESTS OR BUILD_FUNC_TESTS)
    add_library(gtest STATIC IMPORTED GLOBAL)
    set_target_properties(gtest PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${SYSROOT}/include
      IMPORTED_LOCATION             ${SYSROOT}/lib/gtestd.lib
    )

    add_library(gmock STATIC IMPORTED GLOBAL)
    set_target_properties(gmock PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${SYSROOT}/include
      IMPORTED_LOCATION             ${SYSROOT}/lib/Debug/gmock.lib
    )
  endif()
endif()


# Bond Lite subdirectories

include_directories(bondlite/include)

if(BUILD_UNIT_TESTS)
  enable_testing()
  add_subdirectory(bondlite/tests)
endif()


# SDK subdirectories

if(BUILD_HEADERS)
  add_subdirectory(lib/include)
endif()
include_directories(lib/include)

if(BUILD_LIBRARY)
  add_subdirectory(lib)
endif()

if(BUILD_UNIT_TESTS OR BUILD_FUNC_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(BUILD_TEST_TOOL)
  add_subdirectory(linktest)
endif()
