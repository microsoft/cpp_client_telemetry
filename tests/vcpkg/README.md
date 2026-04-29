# Vcpkg Port Integration Tests

End-to-end tests that verify the `mstelemetry` vcpkg port can be installed, found via `find_package(MSTelemetry CONFIG)`, linked, and executed on all supported platforms.

## Prerequisites

1. **vcpkg** — [Install and bootstrap](https://vcpkg.io/en/getting-started):
   ```bash
   git clone https://github.com/microsoft/vcpkg ~/vcpkg
   ~/vcpkg/bootstrap-vcpkg.sh   # Linux/macOS
   ```
   ```powershell
   git clone https://github.com/microsoft/vcpkg C:\vcpkg
   .\vcpkg\bootstrap-vcpkg.bat  # Windows
   ```
2. **Set `VCPKG_ROOT`**:
   ```bash
   export VCPKG_ROOT=~/vcpkg     # Linux/macOS (add to ~/.bashrc)
   ```
   ```powershell
   $env:VCPKG_ROOT = "$HOME\vcpkg"  # Windows (or set system env var)
   ```
3. **Platform tools** — see per-platform sections below.

## Running Tests

All scripts are run from the repo root.

### Windows

**Requires:** Visual Studio 2019+ with C++ workload, cmake

Best run from a **VS Developer Command Prompt** (ensures the same compiler version as vcpkg uses):
```powershell
.\tests\vcpkg\test-vcpkg-windows.ps1 -VcpkgRoot C:\path\to\vcpkg
```

> **Note:** Visual Studio's `vcvarsall.bat` overrides the `VCPKG_ROOT` environment variable.
> Always pass `-VcpkgRoot` explicitly to point at your vcpkg installation.

Optional: specify a different triplet (default auto-detects host architecture):
```powershell
.\tests\vcpkg\test-vcpkg-windows.ps1 -VcpkgRoot C:\path\to\vcpkg -Triplet x64-windows
```

### Linux

**Requires:** gcc/g++, cmake, pkg-config

```bash
./tests/vcpkg/test-vcpkg-linux.sh
```

### macOS

**Requires:** Xcode command line tools, cmake

```bash
./tests/vcpkg/test-vcpkg-macos.sh
```

### iOS

**Requires:** macOS with Xcode + iOS SDK, cmake

**Device build** (cross-compile only — verifies the binary is produced):
```bash
./tests/vcpkg/test-vcpkg-ios.sh
```

**Simulator build** (builds and runs on iOS Simulator):
```bash
./tests/vcpkg/test-vcpkg-ios.sh --simulator
```

The simulator mode uses the built-in vcpkg community triplet `arm64-ios-simulator` that targets the `iphonesimulator` SDK, then executes the binary via `xcrun simctl spawn`.

### Android (cross-compile)

**Requires:** Android NDK, cmake, VCPKG_ROOT set

```bash
# Default: arm64-v8a
./tests/vcpkg/test-vcpkg-android.sh

# Other ABIs:
./tests/vcpkg/test-vcpkg-android.sh armeabi-v7a
./tests/vcpkg/test-vcpkg-android.sh x86_64
```

Set `ANDROID_NDK_HOME` if the script can't find your NDK automatically. Cross-compiled binary can be tested on device via `adb push`/`adb shell`.

## What Gets Tested

Each script runs 3 steps:

1. **Configure** — CMake configures a minimal consumer project with the vcpkg toolchain. Dependencies (including the `mstelemetry` overlay port) are installed automatically.
2. **Build** — The consumer links against `MSTelemetry::mat`
3. **Run** — The test binary exercises `LogManager`, `EventProperties`, and verifies all symbols resolve

## Troubleshooting

| Problem | Fix |
|---------|-----|
| `VCPKG_ROOT is not set` | Set the environment variable to your vcpkg installation |
| vcvarsall overrides `VCPKG_ROOT` | Pass `-VcpkgRoot C:\path\to\vcpkg` explicitly on Windows |
| `RuntimeLibrary mismatch` (Windows) | Run from a VS Developer Command Prompt matching vcpkg's compiler |
| Dependency build fails | Check vcpkg logs in `build-*/consumer/vcpkg-manifest-install.log` |
| iOS SDK not found | Install Xcode and run `xcode-select --install` |
| Android NDK not found | Set `ANDROID_NDK_HOME` to your NDK path (e.g., `$ANDROID_HOME/ndk/26.x.x`) |
| First run is slow | Dependencies (sqlite3, zlib, nlohmann-json, curl) are built on first run |
