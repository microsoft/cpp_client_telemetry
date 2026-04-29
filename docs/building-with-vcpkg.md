# Building 1DS C++ SDK with vcpkg

[vcpkg](https://vcpkg.io/) is a Microsoft cross-platform open source C++ package manager. Onboarding instructions for Windows, Linux and Mac OS X [available here](https://docs.microsoft.com/en-us/cpp/build/vcpkg). This document assumes that the customer build system is already configured to use vcpkg ([getting started guide](https://learn.microsoft.com/en-us/vcpkg/get_started/overview)). 1DS C++ SDK maintainers provide a build recipe, `mstelemetry` port or CONTROL file for vcpkg. The mainline vcpkg repo is refreshed to point to latest stable open source release of 1DS C++ SDK.

## Quick Start

### Installing from the vcpkg registry

Once a new port has been accepted into the official vcpkg registry, install with:

```console
vcpkg install mstelemetry
```

That's it! The package should be compiled for the current OS.

### Installing from the overlay port (development / pre-release)

Before the port is published, or to test local changes, use the overlay port
shipped in this repository:

```console
git clone https://github.com/microsoft/cpp_client_telemetry
cd cpp_client_telemetry
vcpkg install --overlay-ports=tools/ports mstelemetry
```

### Using in your CMake project

After installing, add the SDK to your CMake project:

```cmake
find_package(MSTelemetry CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE MSTelemetry::mat)
```

If you use vcpkg manifest mode (recommended), add `mstelemetry` to your
project's `vcpkg.json`:

```json
{
  "dependencies": ["mstelemetry"]
}
```

## Platform-Specific Instructions

### Windows

```powershell
vcpkg install mstelemetry --triplet=x64-windows-static
```

### Linux

```bash
vcpkg install mstelemetry --triplet=x64-linux
```

### macOS

```bash
# Apple Silicon
vcpkg install mstelemetry --triplet=arm64-osx

# Intel
vcpkg install mstelemetry --triplet=x64-osx
```

### iOS (cross-compile)

```bash
vcpkg install mstelemetry --triplet=arm64-ios
```

### iOS Simulator

vcpkg ships a built-in community triplet for the iOS Simulator:

```bash
vcpkg install mstelemetry --triplet=arm64-ios-simulator
```

See the [vcpkg triplet documentation](https://learn.microsoft.com/en-us/vcpkg/users/triplets)
for details on creating your own custom triplets for other platforms.

### Android (cross-compile)

Requires the Android NDK (`ANDROID_NDK_HOME` must be set):

```bash
vcpkg install mstelemetry --triplet=arm64-android
```

Supported triplets: `arm64-android`, `arm-neon-android`, `x64-android`,
`x86-android`.

## Dependencies

The vcpkg port automatically resolves the following dependencies:

| Dependency     | vcpkg Package   | CMake Target                      | Platforms          |
| -------------- | --------------- | --------------------------------- | ------------------ |
| SQLite3        | `sqlite3`       | `unofficial::sqlite3::sqlite3`    | All                |
| zlib           | `zlib`          | `ZLIB::ZLIB`                      | All                |
| nlohmann JSON  | `nlohmann-json` | `nlohmann_json::nlohmann_json`    | All                |
| libcurl        | `curl[ssl]`     | `CURL::libcurl`                   | Linux              |

Windows, macOS/iOS, and Android use platform-native HTTP clients (WinInet,
NSURLSession, and HttpClient_Android respectively), so curl is not required
on those platforms.

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
vcpkg install zlib-ng mstelemetry --triplet=x64-windows-static
```

With `ZLIB_COMPAT=ON`, zlib-ng installs as a drop-in replacement — it provides
the same `zlib.h` header, the same function names (`deflate`, `inflate`,
`crc32`), and the same `ZLIB::ZLIB` CMake target. All packages that depend on
`zlib` (including `mstelemetry` and any other libraries like `onnxruntime`)
will automatically use the optimized zlib-ng build.

> **Important:** All libraries in the same binary should link against the same
> zlib. When using `ZLIB_COMPAT=ON`, ensure all dependencies resolve to
> zlib-ng rather than mixing stock zlib and zlib-ng.

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

## Troubleshooting

vcpkg build log files are created in
`${VCPKG_INSTALL_DIR}/buildtrees/mstelemetry/`. Review the following logs
if you encounter package installation failures:

| File | Contents |
| ---- | -------- |
| `build-out.log` | Build stdout (compiler output) |
| `build-err.log` | Build stderr (compiler errors/warnings) |
| `config-*.log`  | CMake configure output |

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
