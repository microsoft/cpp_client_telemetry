# Swift Package Manager (xcframework) — prototype

**Status: prototype / not yet validated on macOS.** This is a first-pass scaffold
for distributing the 1DS C++ SDK to Apple app developers via **Swift Package
Manager (SPM)**, the successor to CocoaPods (the CocoaPods trunk goes read-only
on 2 Dec 2026, and there is no official in-repo podspec today).

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
| `tools/apple/build-xcframework.sh` | Builds a static `libmat.a` per Apple slice via `build-ios.sh`, lipo's the simulator archs, and assembles the xcframework with `xcodebuild -create-xcframework` |
| `tools/apple/module.modulemap` | Defines the `ObjCModule` Clang module the Swift layer imports |
| `tools/apple/MATTelemetry-umbrella.h` | Umbrella over the `ODW*.h` headers baked into the xcframework |

## Build (on macOS)

```bash
tools/apple/build-xcframework.sh release
# -> build/apple/MATTelemetry.xcframework
# -> build/apple/MATTelemetry.xcframework.zip   (+ prints the SPM checksum)
swift build      # resolves Package.swift against the local xcframework
```

## Consume

- **Local:** point a sample app at this package directory (path dependency).
- **Released:** in Xcode, *File → Add Packages…* and enter the repo URL once the
  release flow below is in place.

## Release wiring (to make it URL-consumable)

1. Build the xcframework and zip it (the script does both).
2. Attach `MATTelemetry.xcframework.zip` to the GitHub Release for the tag.
3. Switch the `.binaryTarget` in `Package.swift` from `path:` to
   `url:`+`checksum:` (the `compute-checksum` value the script prints).

This mirrors the `vcpkg-release-bump` workflow: a release-triggered job can build
the xcframework, upload it to the Release, and bump the `binaryTarget` URL +
checksum automatically.

## Known gaps / TODO (validate on macOS)

- **macOS / Catalyst / visionOS slices** — only iOS device + simulator are wired
  up in this first pass; add the macOS slice (see section 3 of the script).
- **Conditional modules** — carry over the `moduleExists()` source exclusions
  from `wrappers/swift/Package.swift` (PrivacyGuard / Sanitizer / DataViewer) and
  keep the umbrella header in sync with the headers actually built.
- **Header flattening** — confirm the `ODW*.h` headers reference each other by
  bare name in the flattened `Headers/` layout (adjust the copy step if not).
- **Static-lib path** — verify `out/lib/libmat.a` is the actual artifact name and
  that `-DBUILD_SHARED_LIBS=OFF` yields a static archive for every slice.
- **Code signing** — release xcframeworks are typically signed; add a signing
  step before zipping for distribution.
