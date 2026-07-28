# Embedding 1DS with CMake

Consumers that build the SDK from source with `add_subdirectory()` or
`FetchContent` can link the same target name used by installed/vcpkg builds:

```cmake
set(BUILD_TEST_TOOL OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_FUNC_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_PACKAGE OFF CACHE BOOL "" FORCE)

add_subdirectory(cpp_client_telemetry)
target_link_libraries(your_target PRIVATE MSTelemetry::mat)
```

For a static SDK build, CMake carries the SDK's link dependencies through the
`MSTelemetry::mat` target, so the consuming target should not need to name the
SDK's internal dependencies directly.

## Non-vcpkg dependency selection

When the CPP11 PAL uses the curl HTTP transport outside vcpkg, the SDK normally
calls `find_package(CURL)` and links `CURL::libcurl` when that imported target is
available. Non-vcpkg Linux builds similarly use `find_package()` for zlib and
SQLite unless the minimal bundled SQLite option is enabled.

To make a superbuild choose the dependency implementation (for example, libcurl
built with OpenSSL vs. mbedTLS) without changing the leaf consumer target, define
the desired dependency targets before adding the SDK and point the matching
`MATSDK_*_TARGET` cache variables at them:

```cmake
# Created by your superbuild, package manager, or imported-target wrappers.
add_library(my_curl_target STATIC IMPORTED GLOBAL)   # OpenSSL or mbedTLS curl
add_library(my_zlib_target STATIC IMPORTED GLOBAL)
add_library(my_sqlite_target STATIC IMPORTED GLOBAL)

set(MATSDK_CURL_TARGET my_curl_target CACHE STRING "" FORCE)
set(MATSDK_ZLIB_TARGET my_zlib_target CACHE STRING "" FORCE)
set(MATSDK_SQLITE_TARGET my_sqlite_target CACHE STRING "" FORCE)
add_subdirectory(cpp_client_telemetry)

target_link_libraries(your_target PRIVATE MSTelemetry::mat)
```

If a `MATSDK_*_TARGET` value is empty, the SDK falls back to its existing
dependency discovery for that library.

`MATSDK_ZLIB_TARGET` is intentionally rejected for non-vcpkg WIN32 builds today:
that legacy path includes the SDK's vendored zlib headers, which rename zlib
symbols to `act_z_*`. Use vcpkg mode, the existing Visual Studio project
dependency path, or add a dedicated bundled-zlib CMake path before supplying a
custom WIN32 zlib target.
