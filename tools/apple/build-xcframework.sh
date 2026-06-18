#!/bin/bash
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# PROTOTYPE: build MATTelemetry.xcframework (1DS C++ core + Obj-C wrappers) for
# Apple platforms, for Swift Package Manager distribution.
#
# Run on macOS with Xcode + CMake installed. Usage:
#   tools/apple/build-xcframework.sh [release|debug]
#
# Produces:
#   build/apple/MATTelemetry.xcframework
#   build/apple/MATTelemetry.xcframework.zip   (+ prints the SPM checksum)
#
# Slices built here: iOS device (arm64) and iOS simulator (arm64 + x86_64 fat).
# A macOS slice (and visionOS, Catalyst) can be folded in the same way -- see
# the note in section 3.
#
# NOTE: this is a first-pass scaffold. It has NOT been executed on macOS yet;
# validate on a mac and adjust the static-lib path / header flattening as needed.

set -euo pipefail

CONFIG="${1:-release}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$ROOT/build/apple"
LIB="libmat.a"   # mat target; the Obj-C wrappers compile into it (lib/CMakeLists.txt:217)

# Force a STATIC libmat that includes the Obj-C wrappers, regardless of the
# repo's default library type.
export CMAKE_OPTS="-DBUILD_SHARED_LIBS=OFF -DBUILD_OBJC_WRAPPER=YES ${CMAKE_OPTS:-}"

rm -rf "$OUT"
mkdir -p "$OUT"

# --- 1. Public Obj-C headers + module map (vended by the xcframework) --------
# Flatten the ODW*.h headers + umbrella + modulemap into one Headers dir. The
# module is named `ObjCModule` to match what wrappers/swift sources import.
HDRS="$OUT/Headers"
mkdir -p "$HDRS"
find "$ROOT/wrappers/obj-c" -maxdepth 1 -name 'ODW*.h' ! -name '*_private.h' -exec cp {} "$HDRS/" \;
cp "$ROOT"/wrappers/obj-c/objc_begin.h "$HDRS/"
cp "$ROOT"/wrappers/obj-c/objc_end.h "$HDRS/"
cp "$ROOT"/tools/apple/MATTelemetry-umbrella.h "$HDRS/"
cp "$ROOT"/tools/apple/module.modulemap "$HDRS/"

# --- 2. Build one static lib per (arch, platform) ----------------------------
build_slice() {  # arch platform out-subdir
  local arch="$1" plat="$2" sub="$3"
  echo "=== building $arch / $plat ($CONFIG) ==="
  ( cd "$ROOT" && ./build-ios.sh clean "$CONFIG" "$arch" "$plat" )
  mkdir -p "$OUT/$sub"
  cp "$ROOT/out/lib/$LIB" "$OUT/$sub/$LIB"
}

build_slice arm64  iphoneos        ios-arm64
build_slice arm64  iphonesimulator ios-arm64-sim
build_slice x86_64 iphonesimulator ios-x86_64-sim

# Fat simulator archive (arm64 + x86_64) -- a single xcframework slice cannot
# mix device and simulator, but it can contain multiple archs for one platform.
mkdir -p "$OUT/ios-simulator"
lipo -create "$OUT/ios-arm64-sim/$LIB" "$OUT/ios-x86_64-sim/$LIB" \
     -output "$OUT/ios-simulator/$LIB"

# --- 3. (Optional) macOS slice ------------------------------------------------
# Add a native macOS build (e.g. cmake -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
# -DBUILD_APPLE_HTTP=YES) here and append another `-library .../libmat.a
# -headers "$HDRS"` pair to the xcodebuild call below. Omitted from this first
# pass to keep the prototype focused on iOS.

# --- 4. Assemble the xcframework ---------------------------------------------
rm -rf "$OUT/MATTelemetry.xcframework"
xcodebuild -create-xcframework \
  -library "$OUT/ios-arm64/$LIB"     -headers "$HDRS" \
  -library "$OUT/ios-simulator/$LIB" -headers "$HDRS" \
  -output  "$OUT/MATTelemetry.xcframework"
echo "Created $OUT/MATTelemetry.xcframework"

# --- 5. Zip + checksum for release distribution ------------------------------
( cd "$OUT" && rm -f MATTelemetry.xcframework.zip \
  && zip -qry MATTelemetry.xcframework.zip MATTelemetry.xcframework )
echo "Zipped:  $OUT/MATTelemetry.xcframework.zip"
echo -n "SPM checksum (for Package.swift binaryTarget url: form): "
swift package compute-checksum "$OUT/MATTelemetry.xcframework.zip"
