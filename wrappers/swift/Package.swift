// swift-tools-version: 5.7
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription
import Foundation

let packageDirectory = URL(fileURLWithPath: #filePath).deletingLastPathComponent()

func moduleExists(_ relativePath: String) -> Bool {
    FileManager.default.fileExists(atPath: packageDirectory.appendingPathComponent(relativePath).standardizedFileURL.path)
}

let hasDiagnosticDataViewer = moduleExists("../../lib/modules/dataviewer")
let hasPrivacyGuard = moduleExists("../../lib/modules/privacyguard")
let hasSanitizer = moduleExists("../../lib/modules/sanitizer")

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
    name: "OneDSSwiftWrapper",
    products: [
        // Products define the executables and libraries a package produces, and make them visible to other packages.
        .library(
            name: "OneDSSwift",
            targets: ["OneDSSwift"]),
    ],
    dependencies: [
        // Dependencies declare other packages that this package depends on.
        // .package(url: /* package url */, from: "1.0.0"),
    ],
    targets: [
        // Targets are the basic building blocks of a package. A target can define a module or a test suite.
        // Targets can depend on other targets in this package, and on products in packages this package depends on.
        .target(
            name: "OneDSSwift",
            dependencies: [],
            path: "Sources/OneDSSwift",
            exclude: excludedSources,
            cSettings: [
                .headerSearchPath("../../Modules/")
            ],
            swiftSettings: swiftSettings),
    ]
)
