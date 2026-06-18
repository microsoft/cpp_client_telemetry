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
# NOTE: this is a first-pass scaffold. It has been validated on macOS for iOS
# device + simulator slices; macOS/Catalyst/visionOS slices are still TODO.

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
build_slice() {  # clean-arg arch platform out-subdir
  local clean_arg="$1"
  shift
  local arch="$1" plat="$2" sub="$3"
  echo "=== building $arch / $plat ($CONFIG) ==="
  ( cd "$ROOT" && ./build-ios.sh $clean_arg "$CONFIG" "$arch" "$plat" )
  mkdir -p "$OUT/$sub"
  cp "$ROOT/out/lib/$LIB" "$OUT/$sub/$LIB"
}

build_slice clean arm64  iphoneos        ios-arm64
build_slice ""    arm64  iphonesimulator ios-arm64-sim
build_slice ""    x86_64 iphonesimulator ios-x86_64-sim

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
