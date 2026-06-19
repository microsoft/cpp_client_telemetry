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
//   2. `swift build` validates macOS consumption; for iOS / Mac Catalyst /
//      visionOS, add this package as a local dependency or build the package
//      with the desired Xcode destination.
//
// Release distribution (so consumers can add the repo by URL in Xcode):
//   .github/workflows/spm-release.yml builds and uploads the xcframework,
//   computes the checksum, rewrites the local `.binaryTarget(... path:)` below
//   to `url:`+`checksum:`, and pushes the 3-component SemVer tag that SPM can
//   resolve.

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
        .macCatalyst(.v14),
        .macOS(.v10_15),
        .visionOS(.v1),
    ],
    products: [
        .library(name: "OneDSSwift", targets: ["OneDSSwift"]),
    ],
    targets: [
        // Prebuilt C++ core + Obj-C wrappers. The xcframework's bundled
        // module map vends the Clang module `MATTelemetryObjC` (see
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
        // xcframework. The conditional source exclusions above must stay in sync
        // with the headers baked into the xcframework.
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
                .linkedFramework("CFNetwork", .when(platforms: [.iOS, .macCatalyst, .macOS, .visionOS])),
                .linkedFramework("CoreFoundation", .when(platforms: [.iOS, .macCatalyst, .macOS, .visionOS])),
                .linkedFramework("Foundation", .when(platforms: [.iOS, .macCatalyst, .macOS, .visionOS])),
                .linkedFramework("Network", .when(platforms: [.iOS, .macCatalyst, .macOS, .visionOS])),
                .linkedFramework("SystemConfiguration", .when(platforms: [.iOS, .macCatalyst, .macOS, .visionOS])),
                .linkedFramework("IOKit", .when(platforms: [.macOS])),
                .linkedFramework("UIKit", .when(platforms: [.iOS, .macCatalyst, .visionOS])),
            ]),
    ]
)
