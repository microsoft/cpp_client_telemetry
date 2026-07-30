# Embedding 1DS with CMake

Consumers that build the SDK from source with `add_subdirectory()` or
`FetchContent` can link the same target name used by installed/vcpkg builds:

```cmake
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
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

Use standard `BUILD_SHARED_LIBS=OFF|ON` to select static or shared output.
SDK-specific behavior continues to use namespaced `MATSDK_*` options.

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
unstripped vendored dependency. `SYSTEM` consumes the canonical
`SQLite::SQLite3` / `ZLIB::ZLIB` targets or uses `find_package()`. `AUTO`
preserves platform defaults: system dependencies on desktop/Apple source builds
and vendored dependencies on Windows/Android source builds.

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

To make a superbuild choose dependency implementations without changing the
leaf consumer target, define the standard CMake targets before adding the SDK:

```cmake
# These may be real targets or aliases to targets owned by your superbuild.
add_library(CURL::libcurl ALIAS my_curl_target)
add_library(ZLIB::ZLIB ALIAS my_zlib_target)
add_library(SQLite::SQLite3 ALIAS my_sqlite_target)
add_subdirectory(cpp_client_telemetry)

target_link_libraries(your_target PRIVATE MSTelemetry::mat)
```

For a fully self-contained source build, use `MATSDK_SQLITE_PROVIDER=MINIMAL`
and `MATSDK_ZLIB_PROVIDER=VENDORED`; the vendored targets are PIC, hidden, and
compiled without inheriting the SDK's warnings-as-errors policy.
