# Building 1DS C++ SDK with vcpkg

[vcpkg](https://vcpkg.io/) is a Microsoft cross-platform open source C++ package manager. Onboarding instructions for Windows, Linux and Mac OS X [available here](https://docs.microsoft.com/en-us/cpp/build/vcpkg). This document assumes that the customer build system is already configured to use vcpkg ([getting started guide](https://learn.microsoft.com/en-us/vcpkg/get_started/overview)). The `cpp-client-telemetry` port is published in the official vcpkg registry, so it can be consumed directly with no overlay or extra configuration. Maintainers refresh the registry to point to the latest stable open source release of the 1DS C++ SDK on each release.

The port provides the core SDK — the `MSTelemetry::mat` target and its public
C++ headers. The optional Microsoft-proprietary modules (Privacy Guard,
Sanitizer, ECS/EXP, Compliant-by-Default, AFD, CDS) are not part of the public
package; to use those, build from a source checkout with submodules:

```console
git clone --recurse-submodules https://github.com/microsoft/cpp_client_telemetry
# then build with the repo's own CMake/build scripts
```

## Quick Start

### Installing from the vcpkg registry

The `cpp-client-telemetry` port is available in the [official vcpkg registry](https://github.com/microsoft/vcpkg/tree/master/ports/cpp-client-telemetry),
so you can install it directly — no overlay or extra configuration required:

```console
vcpkg install cpp-client-telemetry
```

That's it! The package should be compiled for the current OS.

### Installing from the overlay port (development / pre-release)

The overlay port shipped in this repository is for **development only** — use it
to test local changes to the port, or a newer SDK revision, before they are
published to the registry:

```console
git clone https://github.com/microsoft/cpp_client_telemetry
cd cpp_client_telemetry
vcpkg install --overlay-ports=tools/ports cpp-client-telemetry
```

### Using in your CMake project

After installing, add the SDK to your CMake project:

```cmake
find_package(MSTelemetry CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE MSTelemetry::mat)
```

If you use vcpkg manifest mode (recommended), add `cpp-client-telemetry` to your
project's `vcpkg.json`:

```json
{
  "dependencies": ["cpp-client-telemetry"]
}
```

## Platform-Specific Instructions

### Windows

```powershell
vcpkg install cpp-client-telemetry --triplet=x64-windows-static
```

### Linux

```bash
vcpkg install cpp-client-telemetry --triplet=x64-linux
```

### macOS

```bash
# Apple Silicon
vcpkg install cpp-client-telemetry --triplet=arm64-osx

# Intel
vcpkg install cpp-client-telemetry --triplet=x64-osx
```

### iOS (cross-compile)

```bash
vcpkg install cpp-client-telemetry --triplet=arm64-ios
```

### iOS Simulator

vcpkg ships a built-in community triplet for the iOS Simulator:

```bash
vcpkg install cpp-client-telemetry --triplet=arm64-ios-simulator
```

See the [vcpkg triplet documentation](https://learn.microsoft.com/en-us/vcpkg/users/triplets)
for details on creating your own custom triplets for other platforms.

### Android (cross-compile)

Requires the Android NDK (`ANDROID_NDK_HOME` must be set). vcpkg's built-in
Android triplets default to Android API 28:

```bash
vcpkg install cpp-client-telemetry --triplet=arm64-android
```

This repo's Android build declares `minSdk 23`. To build the SDK and all
vcpkg dependencies for API 23, create a custom triplet in your **own**
overlay-triplets directory that sets `VCPKG_CMAKE_SYSTEM_VERSION` to `23`. For
example, save the following as `<your-triplets-dir>/arm64-android-api23.cmake`:

```cmake
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Android)
set(VCPKG_CMAKE_SYSTEM_VERSION 23)
set(VCPKG_MAKE_BUILD_TRIPLET "--host=aarch64-linux-android")
set(VCPKG_CMAKE_CONFIGURE_OPTIONS -DANDROID_ABI=arm64-v8a)
```

Then install, pointing `--overlay-triplets` at that directory:

```bash
vcpkg install cpp-client-telemetry \
  --triplet=arm64-android-api23 \
  --overlay-triplets=<your-triplets-dir>
```

Build the whole dependency graph with the same Android API triplet. Mixing
*higher*-API dependencies into a *lower*-API consumer fails to link — the default
`*-android` triplets target API 28, and their objects reference Bionic libc
symbols absent at API 23. The reverse (lower-API deps in a higher-API consumer)
is safe, since Android native code is forward-compatible. (This repo's CI
validates equivalent API 23 triplets under `tests/vcpkg/triplets`, which are test
scaffolding and not part of the published package.)

Supported triplets: `arm64-android`, `arm-neon-android`, `x64-android`,
`x86-android`.

## Dependencies

The vcpkg port automatically resolves the following dependencies:

| Dependency     | vcpkg Package   | CMake Target                      | Platforms          |
| -------------- | --------------- | --------------------------------- | ------------------ |
| SQLite3        | `sqlite3`       | `unofficial::sqlite3::sqlite3`    | Non-Apple (default; see `minimal-sqlite`). **macOS/iOS link the system `libsqlite3`** (`SQLite::SQLite3`) |
| zlib           | `zlib`          | `ZLIB::ZLIB`                      | Non-Apple. **macOS/iOS link the system `libz`** |
| nlohmann JSON  | `nlohmann-json` | `nlohmann_json::nlohmann_json`    | All                |
| libcurl        | `curl[openssl]` or `curl[mbedtls]` | `CURL::libcurl`          | Non-Windows, non-Apple (required; TLS backend selectable: OpenSSL default or mbedTLS) |

On **macOS/iOS** the SDK links the OS-provided `libsqlite3` and `libz` (the same
system libraries the SDK's Swift Package links), so the vcpkg `sqlite3` and `zlib`
packages are not pulled there — those binaries carry no bundled SQLite/zlib.
(`minimal-sqlite` therefore has no effect on Apple.)

The external `sqlite3` package is provided by the default `system-sqlite`
feature. The `minimal-sqlite` feature replaces it with a private, feature-stripped
SQLite built from the SDK's vendored amalgamation — see
[Build a private minimal SQLite](#build-a-private-minimal-sqlite-minimal-sqlite-feature).

libcurl is provided by the default `curl-openssl` feature; `curl-mbedtls` swaps in
the mbedTLS backend — see
[Choose the HTTP client / TLS backend](#choose-the-http-client--tls-backend-largest-lever-on-linux).

Windows and macOS/iOS use platform-native HTTP clients (WinInet and
NSURLSession respectively). Android vcpkg consumers use native libcurl because
the Java-backed `HttpClient_Android` singleton is initialized by the repo's
Android Gradle/AAR flow, not by standalone native vcpkg consumers.

> **Note (Windows):** The port targets the MSVC/`WIN32` PAL on Windows, which
> uses WinInet, so `curl` is declared for `linux | android` only. A MinGW /
> non-MSVC Windows triplet — or forcing `-DPAL_IMPLEMENTATION=CPP11` on Windows —
> selects the curl HTTP client, which the port does not provision on Windows
> (broadening `curl` to `windows` would pull an unused curl into every MSVC
> build, since vcpkg platform expressions can't key off the PAL). Use a standard
> MSVC triplet such as `x64-windows-static` for Windows vcpkg builds.

## Optional: SIMD-Optimized zlib with zlib-ng

The vcpkg port depends on stock `zlib` by default. If you want SIMD-optimized
compression (Intel PCLMULQDQ/AVX2, ARM NEON, etc.), you can transparently
replace zlib with [zlib-ng](https://github.com/zlib-ng/zlib-ng) in
compatibility mode. No changes to the port or SDK code are needed.

**Add one line to your vcpkg triplet file** (e.g., `x64-windows-static.cmake`):

```cmake
set(ZLIB_COMPAT ON)
```

Then install `zlib-ng` instead of `zlib`:

```console
vcpkg install zlib-ng cpp-client-telemetry --triplet=x64-windows-static
```

With `ZLIB_COMPAT=ON`, zlib-ng installs as a drop-in replacement — it provides
the same `zlib.h` header, the same function names (`deflate`, `inflate`,
`crc32`), and the same `ZLIB::ZLIB` CMake target. All packages that depend on
`zlib` (including `cpp-client-telemetry` and any other libraries like `onnxruntime`)
will automatically use the optimized zlib-ng build.

> **Important:** All libraries in the same binary should link against the same
> zlib. When using `ZLIB_COMPAT=ON`, ensure all dependencies resolve to
> zlib-ng rather than mixing stock zlib and zlib-ng.

## Reducing binary footprint

This section applies when the SDK is linked **statically** into your binary
(the default for the `*-static` vcpkg triplets) — most footprint control then
lives on *your* side of the link. If you instead consume a **dynamic** `mat`
(e.g. the default `x64-windows` triplet, or `BUILD_SHARED_LIBS=ON`), the runtime
ships as its own `mat.dll` / `libmat.so` / `libmat.dylib`; the SDK's own
`-fvisibility=hidden` and `/Gy /Gw` already trim its exported symbol table, and
the consumer-side linker options below are specific to the static-link case.

### Enable linker dead-stripping (largest lever)

The SDK is compiled with function-level linking (`/Gy /Gw` on MSVC,
`-ffunction-sections -fdata-sections` on GCC/Clang) so that **your** linker can
discard SDK code you never reference. Make sure your final link enables it:

- **MSVC:** `/OPT:REF` (drop unreferenced functions/data) and `/OPT:ICF` (fold
  identical COMDATs). These are on by default for Release, **but `/DEBUG` flips
  their default to off** (`/OPT:NOREF,NOICF`, per the MSVC `/OPT` docs) — so if
  you ship PDBs, re-enable them explicitly. `/OPT:REF` is also incompatible with
  incremental linking, so set `/INCREMENTAL:NO`:

  ```cmake
  target_link_options(your_target PRIVATE
    $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/OPT:REF>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/OPT:ICF>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/INCREMENTAL:NO>)
  ```

- **GCC / Clang:** link with `-Wl,--gc-sections`.
- **Apple (clang):** link with `-Wl,-dead_strip`.

This is by far the largest lever — on a static `x64-windows-static` Release link
it can roughly halve the binary. The SDK's `/Gy /Gw` flags only *enable* this;
the stripping happens at your link. Keep the SDK a static dependency linked
*into* your binary: if you re-export its API across your own DLL boundary, the
export table pins its symbols and defeats `/OPT:REF`.

### Choose the HTTP client / TLS backend (largest lever on Linux)

On Linux/Android the built-in HTTP client is libcurl, and curl's TLS backend
dominates the SDK's footprint. (Windows uses WinInet and Apple uses NSURLSession,
so this section does not apply there.) The port exposes the TLS backend as two
mutually-exclusive features; pick the one that matches what your application
already has:

| Feature | Transport | Approx. stripped size¹ | Use when |
| ------- | --------- | ---------------------- | -------- |
| `curl-openssl` (default) | libcurl + OpenSSL | ~10.6 MB | your app already links OpenSSL (share it) |
| `curl-mbedtls` | libcurl + mbedTLS | ~4.4 MB | your app has no HTTP/TLS stack of its own |

¹ Rough sizes of a minimal Linux consumer **without** consumer-side dead-stripping
(worst case); enabling `-Wl,--gc-sections` at your link reduces them. Your numbers
depend on triplet, dead-stripping, and what else shares those libraries.

To select **mbedTLS**, two things are required in *your top-level* manifest:

```json
{
  "dependencies": [
    {
      "name": "cpp-client-telemetry",
      "default-features": false,
      "features": [ "minimal-sqlite", "curl-mbedtls" ]
    },
    { "name": "curl", "default-features": false, "features": [ "mbedtls" ] }
  ]
}
```

1. `"default-features": false` (the `[core,...]` form) drops **all** of the SDK's
   default features -- both `curl-openssl` *and* `system-sqlite` -- so the SDK no
   longer *requests* OpenSSL. Because it also drops `system-sqlite`, you must
   re-select a SQLite backend (`minimal-sqlite` above, or `system-sqlite`);
   otherwise the SDK configure step fails with no SQLite feature selected.
2. The explicit top-level `curl` entry is also needed because vcpkg honors curl's
   own `"default-features": false` **only for top-level dependencies** — curl's
   default `ssl` feature (which pulls OpenSSL on Linux) and `non-http` are
   installed transitively otherwise. With both, curl resolves to `curl[core,mbedtls]`
   and OpenSSL is not built; with only the feature, you get
   `curl[mbedtls,ssl,openssl,non-http]` (mbedTLS *and* OpenSSL). This recipe is
   verified with `vcpkg install --dry-run`.

The default install (no features specified) keeps `curl-openssl` and works out of
the box.

### Drop unused SQLite features (json1)

The SDK uses SQLite only for offline event storage — plain tables and indexes,
with no JSON, FTS, R*Tree, or virtual-table features. This in-repo overlay port
already requests `sqlite3` with `default-features: false` on its dependency edge
(the published registry port will follow once this change is upstreamed).

vcpkg unions feature requests across the whole dependency graph, and a
transitive opt-out alone is **not** enough: you must **also** request `sqlite3`
with `default-features: false` in your own top-level manifest to actually omit
`json1` (which compiles SQLite with `SQLITE_OMIT_JSON`, ~50 KB smaller on a
static `x64-windows-static` Release build):

```json
{
  "dependencies": [
    "cpp-client-telemetry",
    { "name": "sqlite3", "default-features": false }
  ]
}
```

If any package in your build (or your own code) needs SQLite's JSON functions,
request `sqlite3[json1]` instead and the extension is restored for the whole
graph.

### Build a private minimal SQLite (`minimal-sqlite` feature)

For a larger, self-contained reduction, the port can compile a private,
feature-stripped SQLite directly from the SDK's vendored amalgamation instead of
linking the external `sqlite3` package at all. The SDK uses SQLite only for its
offline event-storage cache (plain tables and indexes, transactions, WAL,
autovacuum/`VACUUM`, a few PRAGMAs, and one custom UTF-8 SQL function), so this
build omits the unused SQLite subsystems — `SQLITE_OMIT_JSON` plus load-extension,
shared-cache, deprecated APIs, authorization, EXPLAIN, introspection pragmas,
deserialize, and more. The result is **~10% smaller SQLite code** (`.text`) and
**~13% smaller** as a stripped object, and it drops the external `sqlite3`
dependency from your graph entirely.

Enable it through the vcpkg feature:

```json
{
  "dependencies": [
    {
      "name": "cpp-client-telemetry",
      "default-features": false,
      "features": [ "minimal-sqlite" ]
    }
  ]
}
```

Use the `[core,minimal-sqlite]` form (here, `"default-features": false` is the
`[core]` part) so the default `system-sqlite` feature — and its `sqlite3`
dependency — is dropped. Requesting `minimal-sqlite` *without* `[core]` still
pulls in the default `system-sqlite`; that is harmless (the external `sqlite3` is
installed but unused) but does not save the dependency.

For a plain (non-vcpkg) CMake build, pass the option directly:

```bash
cmake -DMATSDK_MINIMAL_SQLITE=ON ..
```

The strip is **amalgamation-safe**: it changes no SQLite grammar/parser, so no
code generation is required. All offline storage features the SDK relies on (WAL,
autovacuum, `VACUUM`, PRAGMAs, the custom UTF-8 function, blobs, 64-bit integers,
transactions) are retained, and the SDK's offline-storage unit tests pass
unchanged against the minimal build.

> **Caveat — symbol visibility when linking statically.** The private SQLite keeps
> SQLite's default `sqlite3_*` symbol names. For a **shared** `mat`
> (`mat.dll` / `libmat.so` / `libmat.dylib`), those symbols are hidden by the
> SDK's `-fvisibility=hidden`, so there is no conflict. For a **static** `mat`,
> the minimal SQLite is installed and exported as a separate
> `MSTelemetry::sqlite3_bundled` archive that links into your binary; if **any**
> part of the final static link — your own code *or another dependency* — also
> pulls in SQLite, the duplicate `sqlite3_*` symbols will collide at link time. In
> that case, prefer the default `system-sqlite` feature so the whole graph shares a
> single SQLite.

## How It Works: MATSDK_USE_VCPKG_DEPS

When the SDK detects it is being built via vcpkg (by checking for
`VCPKG_TOOLCHAIN` or `VCPKG_TARGET_TRIPLET`), it automatically sets
`MATSDK_USE_VCPKG_DEPS=ON`. This switches dependency resolution from
vendored sources to vcpkg-provided packages via `find_package()`.

You can also set this explicitly for custom CMake workflows:

```bash
cmake -DMATSDK_USE_VCPKG_DEPS=ON \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
      ..
```

## Migrating from the older overlay port

Older revisions of this overlay port could build the SDK with the private
`lib/modules` submodules from a local `--recurse-submodules` checkout. The port
now builds the core SDK from a pinned upstream commit, and the
`response_file_*.txt` shortcuts have been removed (use a standard triplet). If
you relied on the old flow for the proprietary modules, build from a
`--recurse-submodules` source checkout instead.

## Troubleshooting

vcpkg build log files are created under your vcpkg root in
`buildtrees/cpp-client-telemetry/` (or wherever `--x-buildtrees-root` points). Review
these logs if you encounter package installation failures:

| File | Contents |
| ---- | -------- |
| `config-<triplet>-out.log` / `config-<triplet>-err.log` | CMake configure stdout/stderr |
| `config-<triplet>-<dbg\|rel>-*.log` | Per-config configure detail (CMakeCache, CMakeConfigureLog, ninja) |
| `install-<triplet>-<dbg\|rel>-out.log` / `-err.log` | Build + install stdout/stderr (compiler output/errors) |
| `stdout-<triplet>.log` | vcpkg per-triplet summary output |

You can also pass `--debug` to `vcpkg install` for verbose diagnostics.

## Testing the Port

Integration tests are provided in `tests/vcpkg/`. Each script builds the
port from the overlay, compiles a test consumer, and runs runtime checks:

```bash
# Windows (from Developer Command Prompt)
.\tests\vcpkg\test-vcpkg-windows.ps1 -VcpkgRoot C:\path\to\vcpkg

# Linux
./tests/vcpkg/test-vcpkg-linux.sh

# macOS
./tests/vcpkg/test-vcpkg-macos.sh

# iOS (cross-compile — verifies binary is produced)
./tests/vcpkg/test-vcpkg-ios.sh

# iOS Simulator (builds and runs on simulator)
./tests/vcpkg/test-vcpkg-ios.sh --simulator

# Android (cross-compile — verifies binary is produced)
./tests/vcpkg/test-vcpkg-android.sh
```

See [tests/vcpkg/README.md](../tests/vcpkg/README.md) for prerequisites and
detailed usage.
