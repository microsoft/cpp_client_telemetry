function(matsdk_bool_option canonical legacy description default_value)
  set(initial_value "${default_value}")
  set(canonical_predefined OFF)
  if(DEFINED ${canonical})
    set(canonical_predefined ON)
  endif()
  set(legacy_defined OFF)
  if(NOT "${legacy}" STREQUAL "")
    if(DEFINED ${legacy})
      set(legacy_defined ON)
      if(NOT canonical_predefined)
        set(initial_value "${${legacy}}")
      endif()
    endif()
  endif()
  option(${canonical} "${description}" "${initial_value}")
  if(canonical_predefined AND legacy_defined
     AND NOT "${${legacy}}" STREQUAL "${${canonical}}")
    message(DEPRECATION
      "${legacy} is deprecated and conflicts with ${canonical}; "
      "${canonical}=${${canonical}} takes precedence.")
  endif()
endfunction()

if(DEFINED PROJECT_IS_TOP_LEVEL)
  set(MATSDK_PROJECT_IS_TOP_LEVEL "${PROJECT_IS_TOP_LEVEL}")
elseif(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(MATSDK_PROJECT_IS_TOP_LEVEL ON)
else()
  set(MATSDK_PROJECT_IS_TOP_LEVEL OFF)
endif()

matsdk_bool_option(MATSDK_BUILD_HEADERS BUILD_HEADERS
  "Build API headers" ON)
matsdk_bool_option(MATSDK_BUILD_LIBRARY BUILD_LIBRARY
  "Build the SDK library" ON)
matsdk_bool_option(MATSDK_BUILD_TEST_TOOL BUILD_TEST_TOOL
  "Build the console test tool" "${MATSDK_PROJECT_IS_TOP_LEVEL}")
matsdk_bool_option(MATSDK_BUILD_UNIT_TESTS BUILD_UNIT_TESTS
  "Build unit tests" "${MATSDK_PROJECT_IS_TOP_LEVEL}")
matsdk_bool_option(MATSDK_BUILD_FUNC_TESTS BUILD_FUNC_TESTS
  "Build functional tests" "${MATSDK_PROJECT_IS_TOP_LEVEL}")
matsdk_bool_option(MATSDK_BUILD_JNI_WRAPPER BUILD_JNI_WRAPPER
  "Build the JNI wrapper" OFF)
matsdk_bool_option(MATSDK_BUILD_OBJC_WRAPPER BUILD_OBJC_WRAPPER
  "Build the Objective-C wrapper" ON)
matsdk_bool_option(MATSDK_BUILD_SWIFT_WRAPPER BUILD_SWIFT_WRAPPER
  "Build Swift wrappers" ON)
matsdk_bool_option(MATSDK_BUILD_PACKAGE BUILD_PACKAGE
  "Build an SDK package" "${MATSDK_PROJECT_IS_TOP_LEVEL}")
matsdk_bool_option(MATSDK_BUILD_PRIVACYGUARD BUILD_PRIVACYGUARD
  "Build Privacy Guard" ON)
matsdk_bool_option(MATSDK_BUILD_CDS BUILD_CDS
  "Build Common Diagnostic Stack" ON)
matsdk_bool_option(MATSDK_BUILD_LIVEEVENTINSPECTOR BUILD_LIVEEVENTINSPECTOR
  "Build Live Event Inspector" ON)
matsdk_bool_option(MATSDK_BUILD_SIGNALS BUILD_SIGNALS
  "Build Signals" ON)
matsdk_bool_option(MATSDK_BUILD_SANITIZER BUILD_SANITIZER
  "Build Sanitizer" ON)
matsdk_bool_option(MATSDK_BUILD_AZMON BUILD_AZMON
  "Build Azure Monitor / Application Insights support" ON)
matsdk_bool_option(MATSDK_BUILD_APPLE_HTTP BUILD_APPLE_HTTP
  "Build the Apple-native HTTP client" "${APPLE}")

set(_matsdk_ios_default OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR CMAKE_SYSTEM_NAME STREQUAL "visionOS")
  set(_matsdk_ios_default ON)
endif()
matsdk_bool_option(MATSDK_BUILD_IOS BUILD_IOS
  "Build for iOS or visionOS" "${_matsdk_ios_default}")

matsdk_bool_option(MATSDK_WARNINGS_AS_ERRORS ""
  "Treat warnings in SDK-owned targets as errors" "${MATSDK_PROJECT_IS_TOP_LEVEL}")
option(LINK_STATIC_DEPENDS
  "Deprecated no-op retained for compatibility with legacy build scripts" ON)

set(_matsdk_library_type_default STATIC)
if(NOT DEFINED MATSDK_LIBRARY_TYPE AND DEFINED BUILD_SHARED_LIBS AND BUILD_SHARED_LIBS)
  set(_matsdk_library_type_default SHARED)
endif()
set(MATSDK_LIBRARY_TYPE "${_matsdk_library_type_default}" CACHE STRING
  "SDK library type: STATIC or SHARED")
set_property(CACHE MATSDK_LIBRARY_TYPE PROPERTY STRINGS STATIC SHARED)
string(TOUPPER "${MATSDK_LIBRARY_TYPE}" MATSDK_LIBRARY_TYPE)
if(NOT MATSDK_LIBRARY_TYPE STREQUAL "STATIC" AND NOT MATSDK_LIBRARY_TYPE STREQUAL "SHARED")
  message(FATAL_ERROR
    "MATSDK_LIBRARY_TYPE must be STATIC or SHARED; got '${MATSDK_LIBRARY_TYPE}'.")
endif()

set(MATSDK_SQLITE_PROVIDER "AUTO" CACHE STRING
  "SQLite dependency provider: AUTO, SYSTEM, MINIMAL, or VENDORED")
set_property(CACHE MATSDK_SQLITE_PROVIDER PROPERTY STRINGS AUTO SYSTEM MINIMAL VENDORED)
set(MATSDK_ZLIB_PROVIDER "AUTO" CACHE STRING
  "zlib dependency provider: AUTO, SYSTEM, or VENDORED")
set_property(CACHE MATSDK_ZLIB_PROVIDER PROPERTY STRINGS AUTO SYSTEM VENDORED)

string(TOUPPER "${MATSDK_SQLITE_PROVIDER}" MATSDK_SQLITE_PROVIDER_RESOLVED)
string(TOUPPER "${MATSDK_ZLIB_PROVIDER}" MATSDK_ZLIB_PROVIDER_RESOLVED)

if(MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "AUTO")
  if(MATSDK_SQLITE_TARGET)
    set(MATSDK_SQLITE_PROVIDER_RESOLVED SYSTEM)
  elseif(NOT MATSDK_USE_VCPKG_DEPS
      AND (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Android"))
    set(MATSDK_SQLITE_PROVIDER_RESOLVED VENDORED)
  else()
    set(MATSDK_SQLITE_PROVIDER_RESOLVED SYSTEM)
  endif()
endif()

if(MATSDK_ZLIB_PROVIDER_RESOLVED STREQUAL "AUTO")
  if(MATSDK_ZLIB_TARGET)
    set(MATSDK_ZLIB_PROVIDER_RESOLVED SYSTEM)
  elseif(NOT MATSDK_USE_VCPKG_DEPS
      AND (WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Android"))
    set(MATSDK_ZLIB_PROVIDER_RESOLVED VENDORED)
  else()
    set(MATSDK_ZLIB_PROVIDER_RESOLVED SYSTEM)
  endif()
endif()

if(NOT MATSDK_SQLITE_PROVIDER_RESOLVED MATCHES "^(SYSTEM|MINIMAL|VENDORED)$")
  message(FATAL_ERROR
    "MATSDK_SQLITE_PROVIDER must be AUTO, SYSTEM, MINIMAL, or VENDORED; "
    "got '${MATSDK_SQLITE_PROVIDER}'.")
endif()
if(NOT MATSDK_ZLIB_PROVIDER_RESOLVED MATCHES "^(SYSTEM|VENDORED)$")
  message(FATAL_ERROR
    "MATSDK_ZLIB_PROVIDER must be AUTO, SYSTEM, or VENDORED; "
    "got '${MATSDK_ZLIB_PROVIDER}'.")
endif()
if(MATSDK_SQLITE_TARGET AND NOT MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "SYSTEM")
  message(FATAL_ERROR
    "MATSDK_SQLITE_TARGET requires MATSDK_SQLITE_PROVIDER=AUTO or SYSTEM.")
endif()
if(MATSDK_ZLIB_TARGET AND NOT MATSDK_ZLIB_PROVIDER_RESOLVED STREQUAL "SYSTEM")
  message(FATAL_ERROR
    "MATSDK_ZLIB_TARGET requires MATSDK_ZLIB_PROVIDER=AUTO or SYSTEM.")
endif()

set(MATSDK_BUNDLE_SQLITE OFF)
if(MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "MINIMAL"
    OR MATSDK_SQLITE_PROVIDER_RESOLVED STREQUAL "VENDORED")
  set(MATSDK_BUNDLE_SQLITE ON)
endif()
set(MATSDK_BUNDLE_ZLIB OFF)
if(MATSDK_ZLIB_PROVIDER_RESOLVED STREQUAL "VENDORED")
  set(MATSDK_BUNDLE_ZLIB ON)
endif()

message(STATUS "MATSDK_LIBRARY_TYPE: ${MATSDK_LIBRARY_TYPE}")
message(STATUS "MATSDK_SQLITE_PROVIDER: ${MATSDK_SQLITE_PROVIDER} -> ${MATSDK_SQLITE_PROVIDER_RESOLVED}")
message(STATUS "MATSDK_ZLIB_PROVIDER: ${MATSDK_ZLIB_PROVIDER} -> ${MATSDK_ZLIB_PROVIDER_RESOLVED}")
