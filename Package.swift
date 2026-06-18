// swift-tools-version: 5.9
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// Swift Package Manager manifest for the 1DS C++ SDK (Microsoft Applications
// Telemetry) on Apple platforms.
//
// PROTOTYPE — distribution model:
//   * The compiled C++ core + Obj-C wrappers ship as a prebuilt binary
//     xcframework (built by tools/apple/build-xcframework.sh). This avoids
//     compiling the CMake/Bond/sqlite/zlib C++ tree through SPM, which is not
//     practical.
//   * The thin Swift wrapper (wrappers/swift/Sources/OneDSSwift) is compiled
//     from source on top of the Obj-C module vended by the xcframework.
//
// Local development:
//   1. Run `tools/apple/build-xcframework.sh release` on macOS with Xcode.
//      It produces ./build/apple/MATTelemetry.xcframework.
//   2. `swift build` (or add this package as a local dependency).
//
// Release distribution (so consumers can add the repo by URL in Xcode):
//   1. Build the xcframework, zip it, and attach it to the GitHub Release.
//   2. Run `swift package compute-checksum MATTelemetry.xcframework.zip`.
//   3. Replace the `.binaryTarget(... path:)` below with the `url:`+`checksum:`
//      form shown in the comment. The vcpkg-release-bump workflow pattern can be
//      extended to automate steps 1-3 on each release tag.

import PackageDescription

let package = Package(
    name: "OneDSSwift",
    platforms: [
        .iOS(.v12),
        .macOS(.v10_15),
    ],
    products: [
        .library(name: "OneDSSwift", targets: ["OneDSSwift"]),
    ],
    targets: [
        // Prebuilt C++ core + Obj-C wrappers. The xcframework's bundled
        // module map vends the Clang module `ObjCModule` (see
        // tools/apple/module.modulemap), which the Swift layer imports.
        //
        // For a tagged release, swap the local path for the hosted artifact:
        //
        //   .binaryTarget(
        //       name: "MATTelemetry",
        //       url: "https://github.com/microsoft/cpp_client_telemetry/releases/download/v3.10.161.1/MATTelemetry.xcframework.zip",
        //       checksum: "<output of swift package compute-checksum>"),
        .binaryTarget(
            name: "MATTelemetry",
            path: "build/apple/MATTelemetry.xcframework"),

        // Thin Swift API layer (source). Depends on the Obj-C module from the
        // xcframework. NOTE: the conditional source exclusions in
        // wrappers/swift/Package.swift (PrivacyGuard / Sanitizer / DataViewer
        // when those private modules aren't built) should be carried over here
        // and kept in sync with the headers baked into the xcframework.
        .target(
            name: "OneDSSwift",
            dependencies: ["MATTelemetry"],
            path: "wrappers/swift/Sources/OneDSSwift"),
    ]
)
