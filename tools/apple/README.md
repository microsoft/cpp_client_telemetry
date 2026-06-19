# Swift Package Manager xcframework prototype

This packages the 1DS C++ SDK for Apple developers through Swift Package Manager
(SPM), using a prebuilt xcframework for the C++ core plus Obj-C wrappers and
compiling the Swift API from source.

## Package shape

SPM is not a good fit for compiling this SDK's full C++ tree directly because
the SDK depends on CMake, Bond codegen, vendored sqlite3/zlib, and platform
conditionals. Instead, the package is split into:

| Layer | Packaging |
| --- | --- |
| C++ core + Obj-C wrappers (`ODW*`) | `MATTelemetry.xcframework` binary target |
| Swift API (`OneDSSwift`) | Source target in `wrappers/swift/Sources/OneDSSwift` |

The xcframework vendors a Clang module named `ObjCModule` through
`tools/apple/module.modulemap` and `MATTelemetry-umbrella.h`, matching the
existing Swift sources' `import ObjCModule`.

## Supported slices

`tools/apple/build-xcframework.sh release` builds:

| Platform | Slice |
| --- | --- |
| iOS device | `ios-arm64` |
| iOS Simulator | `ios-arm64_x86_64-simulator` |
| Mac Catalyst | `ios-arm64_x86_64-maccatalyst` |
| macOS | `macos-arm64_x86_64` |
| visionOS device | `xros-arm64` |
| visionOS Simulator | `xros-arm64-simulator` |

## Important files

| File | Purpose |
| --- | --- |
| `Package.swift` | Root SPM manifest: binary target + Swift source target |
| `tools/apple/build-xcframework.sh` | Builds static `libmat.a` slices and assembles `MATTelemetry.xcframework` |
| `tools/apple/module.modulemap` | Defines the `ObjCModule` Clang module |
| `tools/apple/MATTelemetry-umbrella.h` | Base umbrella for always-available Obj-C wrapper headers |
| `tools/apple/MATTelemetryAvailability.json` | Optional-module manifest consumed by `Package.swift` |
| `.github/workflows/spm-release.yml` | Release automation for the hosted xcframework and SPM tag |

## Local build

Run on macOS with Xcode and CMake:

```bash
tools/apple/build-xcframework.sh release
# -> build/apple/MATTelemetry.xcframework
# -> build/apple/MATTelemetry.xcframework.zip
# -> prints the SwiftPM checksum
```

For local development, `Package.swift` points at
`build/apple/MATTelemetry.xcframework`. `swift build` validates macOS
consumption; use Xcode destinations for iOS Simulator, Mac Catalyst, and
visionOS Simulator.

## Consumption

- **Local:** add this repository as a local package dependency after building
  `build/apple/MATTelemetry.xcframework`.
- **Released:** add the repository URL in Xcode and pin the parallel
  3-component SemVer tag published by the release workflow:

```swift
.package(url: "https://github.com/microsoft/cpp_client_telemetry.git", from: "3.10.161")
```

The SDK's native release tags are 4-component (`vX.Y.Z.W`), which SPM does not
accept as SemVer. The release workflow publishes the corresponding `X.Y.Z` tag.

## Release workflow

`.github/workflows/spm-release.yml` runs for published SDK releases and manual
dispatch. It:

1. Builds and zips `MATTelemetry.xcframework`.
2. Validates package consumption for macOS, iOS Simulator, Mac Catalyst, and
   visionOS Simulator.
3. Uploads the zip to the GitHub Release.
4. Rewrites `Package.swift` from local `path:` to hosted `url:` + `checksum:`.
5. Commits the resolved manifest and pushes the 3-component SPM tag.

The private `lib/modules` submodule is intentionally not fetched by the release
workflow, so optional module headers and Swift sources are gated by
`MATTelemetryAvailability.json`.

## Validation performed

- Full xcframework build for iOS, iOS Simulator, Mac Catalyst, macOS, visionOS,
  and visionOS Simulator.
- SwiftPM/Xcode builds for macOS, iOS Simulator, Mac Catalyst, visionOS
  Simulator, and visionOS device.
- External TelemetryTest package consumer builds, including visionOS.
- Obj-C module/static-link smoke tests for representative binary slices.
- Apple Vision Pro simulator runtime installation and boot.

## Known gaps

- Release xcframework signing/notarization.
- End-to-end execution of `.github/workflows/spm-release.yml` on a real
  published release.
