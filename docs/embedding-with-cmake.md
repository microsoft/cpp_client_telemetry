# Embedding 1DS with CMake

Consumers that build the SDK from source with `add_subdirectory()` or
`FetchContent` can link the same target name used by installed/vcpkg builds:

```cmake
set(MATSDK_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)
set(MATSDK_BUILD_TEST_TOOL OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_FUNC_TESTS OFF CACHE BOOL "" FORCE)
set(MATSDK_BUILD_PACKAGE OFF CACHE BOOL "" FORCE)

add_subdirectory(cpp_client_telemetry)
target_link_libraries(your_target PRIVATE MSTelemetry::mat)
```

For a static SDK build, CMake carries the SDK's link dependencies through the
`MSTelemetry::mat` target, so the consuming target should not need to name the
SDK's internal dependencies directly.

`MATSDK_LIBRARY_TYPE` explicitly selects `STATIC` or `SHARED` without changing
the parent project's global `BUILD_SHARED_LIBS` value. Legacy `BUILD_*` inputs
remain accepted for compatibility, but new integrations should use the
namespaced `MATSDK_*` options.

`MATSDK_WARNINGS_AS_ERRORS` defaults to `ON` for standalone SDK builds and
`OFF` when the SDK is embedded. Its warning policy is private to SDK-owned
targets and never propagates to the parent consumer or vendored dependencies.
Set it explicitly to `ON` in consumer CI to test new toolchains strictly.

## SQLite and zlib providers

Source builds can select dependency modes without patching 1DS sources:

```cmake
set(MATSDK_SQLITE_PROVIDER MINIMAL CACHE STRING "" FORCE) # SYSTEM, MINIMAL, VENDORED
set(MATSDK_ZLIB_PROVIDER VENDORED CACHE STRING "" FORCE)  # SYSTEM or VENDORED
```

`MINIMAL` builds the feature-stripped SQLite amalgamation. `VENDORED` builds the
unstripped vendored dependency. `SYSTEM` uses `find_package()` unless a matching
`MATSDK_SQLITE_TARGET` or `MATSDK_ZLIB_TARGET` is supplied. `AUTO` preserves
platform defaults: system dependencies on desktop/Apple source builds and
vendored dependencies on Windows/Android source builds.

## Non-vcpkg dependency selection

When the CPP11 PAL uses the curl HTTP transport outside vcpkg, the SDK normally
calls `find_package(CURL)` and links `CURL::libcurl` when that imported target is
available. On Linux, set `MATSDK_CURL_PROVIDER=FETCH` to let the SDK download and
build a pinned static curl dependency instead:

```cmake
set(MATSDK_CURL_PROVIDER FETCH CACHE STRING "" FORCE)
set(MATSDK_CURL_TLS_BACKEND MBEDTLS CACHE STRING "" FORCE) # or OPENSSL
add_subdirectory(cpp_client_telemetry)

target_link_libraries(your_target PRIVATE MSTelemetry::mat)
```

The default fetched backend is mbedTLS and is fully self-contained. Selecting
`OPENSSL` builds curl from source but still requires the parent build environment
to provide OpenSSL through `find_package(OpenSSL)`.

Non-vcpkg Linux builds similarly use `find_package()` for zlib and SQLite unless
an explicit vendored/minimal provider is selected.

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

For a fully self-contained source build, use `MATSDK_SQLITE_PROVIDER=MINIMAL`
and `MATSDK_ZLIB_PROVIDER=VENDORED`; the vendored targets are PIC, hidden, and
compiled without inheriting the SDK's warnings-as-errors policy.
