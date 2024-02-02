// swift-tools-version: 5.8
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "SamplePackage",
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "SamplePackage",
            targets: ["SamplePackage"]),
    ],
    dependencies: [
        .package(path: "../../../wrappers/swift"),
    ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.
        .target(
            name: "SamplePackage",
            dependencies: [
                .product(name: "OneDSSwift", package: "swift"),
            ],
            swiftSettings: [
                .unsafeFlags(["-Xcc", "-I../../../wrappers/swift/Modules/"]),
            ],
            linkerSettings: [
                .unsafeFlags(["-L/usr/local/lib"]),
                // Libs to be linked.
                .linkedLibrary("mat"),
                .linkedLibrary("sqlite3"),
                .linkedLibrary("z"),
                // Frameworks to be linked.
                .linkedFramework("Network"),
                .linkedFramework("SystemConfiguration"),
            ]),
        .testTarget(
            name: "SampleTests",
            dependencies: [
                "SamplePackage",
            ],
            swiftSettings: [
                .unsafeFlags(["-Xcc", "-I../../../wrappers/swift/Modules/"])
            ]),
    ]
)
