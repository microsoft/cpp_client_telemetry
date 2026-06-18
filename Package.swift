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
import Foundation

let packageDirectory = URL(fileURLWithPath: #filePath).deletingLastPathComponent()

func readAvailability() -> [String: Bool] {
    let candidates = [
        "build/apple/MATTelemetryAvailability.json",
        "tools/apple/MATTelemetryAvailability.json",
    ]

    for relativePath in candidates {
        let url = packageDirectory.appendingPathComponent(relativePath).standardizedFileURL
        guard let data = try? Data(contentsOf: url),
              let object = try? JSONSerialization.jsonObject(with: data) as? [String: Bool] else {
            continue
        }
        return object
    }

    return [:]
}

let availability = readAvailability()
let hasDiagnosticDataViewer = availability["diagnosticDataViewer"] ?? false
let hasPrivacyGuard = availability["privacyGuard"] ?? false
let hasSanitizer = availability["sanitizer"] ?? false

var excludedSources: [String] = []
var swiftSettings: [SwiftSetting] = []

if !hasDiagnosticDataViewer {
    excludedSources.append("DiagnosticDataViewer.swift")
}

if hasPrivacyGuard {
    swiftSettings.append(.define("MATSDK_PRIVACYGUARD_AVAILABLE"))
} else {
    excludedSources.append(contentsOf: [
        "CommonDataContext.swift",
        "PrivacyGuard.swift",
        "PrivacyGuardInitConfig.swift",
    ])
}

if !hasSanitizer {
    excludedSources.append(contentsOf: [
        "Sanitizer.swift",
        "SanitizerInitConfig.swift",
    ])
}

let package = Package(
    name: "OneDSSwift",
    platforms: [
        .iOS(.v12),
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
            path: "wrappers/swift/Sources/OneDSSwift",
            exclude: excludedSources,
            swiftSettings: swiftSettings,
            linkerSettings: [
                .linkedLibrary("c++"),
                .linkedLibrary("sqlite3"),
                .linkedLibrary("z"),
                .linkedFramework("CFNetwork", .when(platforms: [.iOS])),
                .linkedFramework("CoreFoundation", .when(platforms: [.iOS])),
                .linkedFramework("Foundation", .when(platforms: [.iOS])),
                .linkedFramework("Network", .when(platforms: [.iOS])),
                .linkedFramework("SystemConfiguration", .when(platforms: [.iOS])),
                .linkedFramework("UIKit", .when(platforms: [.iOS])),
            ]),
    ]
)
