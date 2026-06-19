# Swift Package Manager (xcframework) — prototype

**Status: validated prototype.** This is a first-pass scaffold for distributing
the 1DS C++ SDK to Apple app developers via **Swift Package Manager (SPM)**, the
successor to CocoaPods (the CocoaPods trunk goes read-only on 2 Dec 2026, and
there is no official in-repo podspec today).

## Approach

SPM cannot practically compile this SDK's C++ tree from source (CMake build,
Bond codegen, vendored sqlite3/zlib, heavy platform conditionals). So:

| Layer | How it ships |
| --- | --- |
| C++ core + Obj-C wrappers (`ODW*`) | **Prebuilt binary** — `MATTelemetry.xcframework` (`.binaryTarget`) |
| Swift API (`OneDSSwift`) | **Source** — `wrappers/swift/Sources/OneDSSwift`, depends on the Obj-C module from the xcframework |

The Obj-C wrappers already compile into `libmat.a` on Apple
(`lib/CMakeLists.txt:217`), and the Swift sources already `import ObjCModule`,
so the xcframework just needs to vend a Clang module named `ObjCModule`
(`tools/apple/module.modulemap` + `MATTelemetry-umbrella.h`).

## Files

| File | Purpose |
| --- | --- |
| `Package.swift` (repo root) | Distributable SPM manifest: `binaryTarget` (xcframework) + `OneDSSwift` source target |
| `tools/apple/build-xcframework.sh` | Builds a static `libmat.a` per Apple slice, lipo's the simulator/Catalyst/macOS archs where needed, and assembles the xcframework with `xcodebuild -create-xcframework` |
| `tools/apple/module.modulemap` | Defines the `ObjCModule` Clang module the Swift layer imports |
| `tools/apple/MATTelemetry-umbrella.h` | Umbrella over the `ODW*.h` headers baked into the xcframework |
| `tools/apple/MATTelemetryAvailability.json` | Build-time optional-module manifest consumed by `Package.swift` so Swift sources match the xcframework contents |

## Build (on macOS)

```bash
tools/apple/build-xcframework.sh release
# -> build/apple/MATTelemetry.xcframework
# -> build/apple/MATTelemetry.xcframework.zip   (+ prints the SPM checksum)
swift build      # resolves Package.swift against the local xcframework
```

## Consume

- **Local:** point a sample app at this package directory (path dependency).
- **Released:** in Xcode *File -> Add Package Dependencies...*, enter the repo
  URL and pick a version. SPM only accepts **3-component SemVer**, and the SDK's
  own `vX.Y.Z.W` tags are not valid SemVer, so consumers pin the **parallel
  3-component tag** the release workflow publishes:

  ```swift
  .package(url: "https://github.com/microsoft/cpp_client_telemetry.git", from: "3.10.161")
  ```

## Release wiring

`.github/workflows/spm-release.yml` automates distribution on each published
release (a 4-component `vX.Y.Z.W` tag). On a macOS runner it:

1. Builds `MATTelemetry.xcframework` and zips it.
2. Uploads the zip to the GitHub Release.
3. Computes the SPM checksum and rewrites the `Package.swift` `binaryTarget`
   from `path:` to `url:`+`checksum:`.
4. Commits that manifest and pushes a **3-component SemVer tag** (`X.Y.Z`,
   derived by dropping the trailing build component) that SPM can resolve.

This mirrors the `vcpkg-release-bump` workflow. It requires the root
`Package.swift` to already exist at the release tag (i.e. this prototype merged
before the release is cut).

## Validation performed

- `tools/apple/build-xcframework.sh release` builds the iOS device, iOS
  simulator, Mac Catalyst, visionOS device, visionOS simulator, and macOS
  slices, and prints the SPM checksum.
- `swift build` validates local macOS SwiftPM consumption.
- `xcodebuild -scheme OneDSSwift -destination 'generic/platform=iOS Simulator' build`
  validates iOS Simulator SwiftPM consumption.
- `xcodebuild -scheme OneDSSwift -destination 'platform=macOS,variant=Mac Catalyst' build`
  validates Mac Catalyst SwiftPM consumption.
- `xcodebuild -scheme OneDSSwift -destination 'generic/platform=visionOS Simulator' build`
  validates visionOS Simulator SwiftPM consumption.
- Small Obj-C module/static-link smoke tests validate binary module linkability.

## Known gaps / TODO

- **Code signing** — release xcframeworks are typically signed; add a signing
  step before zipping for distribution.
- **Release workflow validation** — exercise `.github/workflows/spm-release.yml`
  end-to-end on an actual published release.
