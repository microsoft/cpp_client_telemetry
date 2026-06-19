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
# Slices built here: iOS device (arm64), iOS simulator (arm64 + x86_64 fat),
# Mac Catalyst (arm64 + x86_64 fat), visionOS device/simulator (arm64), and
# macOS (arm64 + x86_64 universal).
#
# NOTE: this is a first-pass scaffold. It has been validated on macOS for iOS
# device, simulator, Mac Catalyst, visionOS, and macOS slices.

set -euo pipefail

CONFIG="${1:-release}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$ROOT/build/apple"
LIB="libmat.a"   # mat target; the Obj-C wrappers compile into it.

case "$CONFIG" in
  release) CMAKE_BUILD_TYPE="Release" ;;
  debug) CMAKE_BUILD_TYPE="Debug" ;;
  *)
    echo "Usage: $0 [release|debug]" >&2
    exit 1
    ;;
esac

# Build only the static libmat archive with Obj-C wrappers; slice builds do not
# need the repo's test, Swift wrapper, or package targets.
CMAKE_OPTS="${CMAKE_OPTS:-}"
CMAKE_OPTS="$CMAKE_OPTS -DBUILD_SHARED_LIBS=OFF"
CMAKE_OPTS="$CMAKE_OPTS -DBUILD_OBJC_WRAPPER=YES"
CMAKE_OPTS="$CMAKE_OPTS -DBUILD_TEST_TOOL=OFF"
CMAKE_OPTS="$CMAKE_OPTS -DBUILD_UNIT_TESTS=OFF"
CMAKE_OPTS="$CMAKE_OPTS -DBUILD_FUNC_TESTS=OFF"
CMAKE_OPTS="$CMAKE_OPTS -DBUILD_SWIFT_WRAPPER=OFF"
CMAKE_OPTS="$CMAKE_OPTS -DBUILD_PACKAGE=OFF"
export CMAKE_OPTS

rm -rf "$OUT"
mkdir -p "$OUT"

# --- 1. Public Obj-C headers + module map (vended by the xcframework) --------
# Flatten the ODW*.h headers + umbrella + modulemap into one Headers dir. The
# module is named `ObjCModule` to match what wrappers/swift sources import.
HDRS="$OUT/Headers"
mkdir -p "$HDRS"

has_dataviewer=false
has_privacyguard=false
has_sanitizer=false

cmake_option_enabled() { # option-name default-value
  local option="$1"
  local value="$2"
  local token
  for token in $CMAKE_OPTS; do
    case "$token" in
      -D${option}=*) value="${token#*=}" ;;
      -D${option}) value=ON ;;
    esac
  done
  value="$(printf '%s' "$value" | tr '[:upper:]' '[:lower:]')"
  case "$value" in
    0|false|no|off) return 1 ;;
    *) return 0 ;;
  esac
}

[[ -d "$ROOT/lib/modules/dataviewer" ]] && has_dataviewer=true
if [[ -d "$ROOT/lib/modules/privacyguard" ]] && cmake_option_enabled BUILD_PRIVACYGUARD ON; then
  has_privacyguard=true
fi
if [[ -d "$ROOT/lib/modules/sanitizer" ]] && cmake_option_enabled BUILD_SANITIZER ON; then
  has_sanitizer=true
fi

cat > "$OUT/MATTelemetryAvailability.json" <<EOF
{
  "diagnosticDataViewer": $has_dataviewer,
  "privacyGuard": $has_privacyguard,
  "sanitizer": $has_sanitizer
}
EOF
if [[ "${GITHUB_ACTIONS:-}" == "true" || "${MATTELEMETRY_UPDATE_PACKAGE_AVAILABILITY:-}" == "1" ]]; then
  cp "$OUT/MATTelemetryAvailability.json" "$ROOT/tools/apple/MATTelemetryAvailability.json"
fi

headers=(
  ODWCommonDataContext.h
  ODWEventProperties.h
  ODWLogConfiguration.h
  ODWLogger.h
  ODWLogManager.h
  ODWPrivacyGuardInitConfig.h
  ODWSanitizerInitConfig.h
  ODWSemanticContext.h
)
[[ "$has_dataviewer" == true ]] && headers+=(ODWDiagnosticDataViewer.h)
[[ "$has_privacyguard" == true ]] && headers+=(ODWPrivacyGuard.h)
[[ "$has_sanitizer" == true ]] && headers+=(ODWSanitizer.h)

header_paths=()
for header in "${headers[@]}"; do
  header_paths+=("$ROOT/wrappers/obj-c/$header")
done

cp "${header_paths[@]}" "$HDRS/"
cp "$ROOT"/wrappers/obj-c/objc_begin.h "$HDRS/"
cp "$ROOT"/wrappers/obj-c/objc_end.h "$HDRS/"
cp "$ROOT"/tools/apple/module.modulemap "$HDRS/"

cp "$ROOT"/tools/apple/MATTelemetry-umbrella.h "$HDRS/"
{
  [[ "$has_dataviewer" == true ]] && echo '#import "ODWDiagnosticDataViewer.h"'
  [[ "$has_privacyguard" == true ]] && echo '#import "ODWPrivacyGuard.h"'
  [[ "$has_sanitizer" == true ]] && echo '#import "ODWSanitizer.h"'
} >> "$HDRS/MATTelemetry-umbrella.h"

# --- 2. Build one static lib per (arch, platform) ----------------------------
build_slice() {  # arch platform out-subdir
  local arch="$1" plat="$2" sub="$3"
  echo "=== building $arch / $plat ($CONFIG) ==="
  (
    cd "$ROOT"
    rm -f CMakeCache.txt *.cmake
    rm -rf out
    MATTELEMETRY_SKIP_PACKAGE=1 ./build-ios.sh "$CONFIG" "$arch" "$plat"
  )
  mkdir -p "$OUT/$sub"
  cp "$ROOT/out/lib/$LIB" "$OUT/$sub/$LIB"
}

build_slice arm64  iphoneos        ios-arm64
build_slice arm64  iphonesimulator ios-arm64-sim
build_slice x86_64 iphonesimulator ios-x86_64-sim
build_slice arm64  maccatalyst     maccatalyst-arm64
build_slice x86_64 maccatalyst     maccatalyst-x86_64
build_slice arm64  xros            visionos-arm64
build_slice arm64  xrsimulator     visionos-arm64-sim

# Fat simulator archive (arm64 + x86_64) -- a single xcframework slice cannot
# mix device and simulator, but it can contain multiple archs for one platform.
mkdir -p "$OUT/ios-simulator"
lipo -create "$OUT/ios-arm64-sim/$LIB" "$OUT/ios-x86_64-sim/$LIB" \
     -output "$OUT/ios-simulator/$LIB"

# Fat Catalyst archive (arm64 + x86_64), emitted as a separate platform variant
# from both iOS simulator and native macOS.
mkdir -p "$OUT/maccatalyst"
lipo -create "$OUT/maccatalyst-arm64/$LIB" "$OUT/maccatalyst-x86_64/$LIB" \
     -output "$OUT/maccatalyst/$LIB"

# Native universal macOS archive. Build only the `mat` target in an isolated
# CMake build directory so switching away from the iOS toolchain does not
# disturb the already-copied iOS archives.
echo "=== building arm64+x86_64 / macosx ($CONFIG) ==="
MACOS_BUILD="$OUT/macos-build"
MACOS_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-10.15}"
cmake -S "$ROOT" -B "$MACOS_BUILD" \
  -DMAC_ARCH=universal \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOS_DEPLOYMENT_TARGET" \
  -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
  -DCMAKE_PACKAGE_TYPE=tgz \
  -DBUILD_TEST_TOOL=OFF \
  -DBUILD_UNIT_TESTS=OFF \
  -DBUILD_FUNC_TESTS=OFF \
  -DBUILD_SWIFT_WRAPPER=OFF \
  $CMAKE_OPTS
cmake --build "$MACOS_BUILD" --target mat
mkdir -p "$OUT/macos-universal"
cp "$MACOS_BUILD/lib/$LIB" "$OUT/macos-universal/$LIB"

# --- 3. Assemble the xcframework ---------------------------------------------
rm -rf "$OUT/MATTelemetry.xcframework"
xcodebuild -create-xcframework \
  -library "$OUT/ios-arm64/$LIB"     -headers "$HDRS" \
  -library "$OUT/ios-simulator/$LIB" -headers "$HDRS" \
  -library "$OUT/maccatalyst/$LIB"   -headers "$HDRS" \
  -library "$OUT/visionos-arm64/$LIB" -headers "$HDRS" \
  -library "$OUT/visionos-arm64-sim/$LIB" -headers "$HDRS" \
  -library "$OUT/macos-universal/$LIB" -headers "$HDRS" \
  -output  "$OUT/MATTelemetry.xcframework"
echo "Created $OUT/MATTelemetry.xcframework"

# --- 4. Zip + checksum for release distribution ------------------------------
( cd "$OUT" && rm -f MATTelemetry.xcframework.zip \
  && zip -qry MATTelemetry.xcframework.zip MATTelemetry.xcframework )
echo "Zipped:  $OUT/MATTelemetry.xcframework.zip"
echo -n "SPM checksum (for Package.swift binaryTarget url: form): "
swift package compute-checksum "$OUT/MATTelemetry.xcframework.zip"
