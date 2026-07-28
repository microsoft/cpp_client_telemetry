#!/bin/bash
# Test script: Verify cpp-client-telemetry vcpkg port for iOS
# Usage: ./tests/vcpkg/test-vcpkg-ios.sh [--simulator]
# Prerequisites: VCPKG_ROOT set, macOS with Xcode + iOS SDK, cmake
#
# By default builds for arm64-ios (device). With --simulator, builds for
# the iOS Simulator and runs the binary via xcrun simctl.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OVERLAY_PORTS="${REPO_ROOT}/tools/ports"

# Build the working tree under review (not a pinned release) so this test
# validates the actual SDK source together with the port manifest/portfile.
export MATSDK_VCPKG_SOURCE_DIR="${REPO_ROOT}"
USE_SIMULATOR=false

for arg in "$@"; do
  case "$arg" in
    --simulator) USE_SIMULATOR=true ;;
    *) echo "Unknown option: $arg"; exit 1 ;;
  esac
done

if [ "${USE_SIMULATOR}" = true ]; then
  TRIPLET="arm64-ios-simulator"
  BUILD_DIR="${SCRIPT_DIR}/build-iossim"
  APPLE_SDK="iphonesimulator"
  echo "=== MSTelemetry vcpkg port test (iOS Simulator) ==="
else
  TRIPLET="arm64-ios"
  BUILD_DIR="${SCRIPT_DIR}/build-ios"
  APPLE_SDK="iphoneos"
  echo "=== MSTelemetry vcpkg port test (iOS device cross-compile) ==="
fi

echo "Repository root: ${REPO_ROOT}"

# Check prerequisites
if [ -z "${VCPKG_ROOT}" ]; then
  echo "ERROR: VCPKG_ROOT is not set. Please set it to your vcpkg installation directory."
  exit 1
fi

if [ "$(uname)" != "Darwin" ]; then
  echo "ERROR: iOS builds require macOS with Xcode installed."
  exit 1
fi

HAS_ARM64_HARDWARE="$(sysctl -n hw.optional.arm64 2>/dev/null || echo 0)"
if [ "${USE_SIMULATOR}" = true ] && [ "$(uname -m)" != "arm64" ] && [ "${HAS_ARM64_HARDWARE}" != "1" ]; then
  echo "ERROR: --simulator builds arm64-ios-simulator and requires an Apple Silicon macOS host."
  echo "       Use the default device cross-compile mode on Intel macOS."
  exit 1
fi

if ! command -v python3 &> /dev/null; then
  echo "ERROR: python3 is required for simulator device detection but was not found."
  exit 1
fi

# Verify Xcode SDK
if [ "${USE_SIMULATOR}" = true ]; then
  xcrun --sdk iphonesimulator --show-sdk-path > /dev/null 2>&1 || {
    echo "ERROR: iOS Simulator SDK not found. Install Xcode and iOS Simulator runtime."
    exit 1
  }
else
  xcrun --sdk iphoneos --show-sdk-path > /dev/null 2>&1 || {
    echo "ERROR: iOS SDK not found. Install Xcode and iOS SDK."
    exit 1
  }
fi

VCPKG_TOOLCHAIN="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "${VCPKG_TOOLCHAIN}" ]; then
  echo "ERROR: vcpkg toolchain not found at ${VCPKG_TOOLCHAIN}"
  exit 1
fi

echo "Triplet: ${TRIPLET}"
echo "Apple SDK: ${APPLE_SDK}"

# Clean previous build
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

echo ""
echo "--- Step 1: Configure (vcpkg installs deps automatically) ---"
cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}/consumer" \
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_TOOLCHAIN}" \
  -DVCPKG_TARGET_TRIPLET="${TRIPLET}" \
  -DVCPKG_OVERLAY_PORTS="${OVERLAY_PORTS}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT="${APPLE_SDK}" \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0

echo ""
echo "--- Step 2: Build test consumer for iOS ---"
cmake --build "${BUILD_DIR}/consumer" --config Release

echo ""
echo "--- Step 3: Verify output ---"
BINARY=$(find "${BUILD_DIR}/consumer" -name "vcpkg_test" -type f 2>/dev/null | head -1)
if [ -z "${BINARY}" ]; then
  echo "[FAIL] iOS binary not found"
  exit 1
fi

echo "[PASS] iOS binary produced: ${BINARY}"
file "${BINARY}"

if [ "${USE_SIMULATOR}" = true ]; then
  echo ""
  echo "--- Step 4: Run on iOS Simulator ---"

  # Find an available iPhone simulator
  DEVICE_UDID=$(xcrun simctl list devices available -j \
    | python3 -c "
import sys, json
data = json.load(sys.stdin)['devices']
for runtime, devices in data.items():
    if 'iOS' not in runtime:
        continue
    for d in devices:
        if d.get('isAvailable') and 'iPhone' in d.get('name', ''):
            print(d['udid'])
            sys.exit(0)
print('')
")

  if [ -z "${DEVICE_UDID}" ]; then
    echo "ERROR: No available iPhone simulator found."
    echo "Create one in Xcode's Devices and Simulators window, or use:"
    echo "  xcrun simctl list devicetypes"
    echo "  xcrun simctl list runtimes"
    echo "  xcrun simctl create <name> <device-type-identifier> <runtime-identifier>"
    exit 1
  fi

  echo "Using simulator: ${DEVICE_UDID}"

  # Boot the simulator (ignore error if already booted)
  xcrun simctl boot "${DEVICE_UDID}" 2>/dev/null || true

  # Run the test binary on the simulator
  echo "Executing vcpkg_test on simulator..."
  if xcrun simctl spawn "${DEVICE_UDID}" "${BINARY}"; then
    echo "[PASS] iOS Simulator execution succeeded"
  else
    EXIT_CODE=$?
    echo "[FAIL] iOS Simulator execution failed with exit code ${EXIT_CODE}"
    exit ${EXIT_CODE}
  fi
else
  echo ""
  echo "NOTE: Device binary cannot run on macOS host."
  echo "      Use --simulator to build and run on the iOS Simulator."
fi

echo ""
echo "=== iOS vcpkg port test PASSED ==="
