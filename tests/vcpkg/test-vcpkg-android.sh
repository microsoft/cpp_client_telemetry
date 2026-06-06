#!/bin/bash
# Test script: Verify mstelemetry vcpkg port for Android (cross-compile only)
# Usage: ./tests/vcpkg/test-vcpkg-android.sh [ABI]
#   ABI: arm64-v8a (default), armeabi-v7a, x86_64, x86
# Prerequisites: VCPKG_ROOT set, ANDROID_NDK_HOME set, cmake
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OVERLAY_PORTS="${REPO_ROOT}/tools/ports"

# Android ABI (default: arm64-v8a)
ANDROID_ABI="${1:-arm64-v8a}"

# Map ABI to vcpkg triplet
case "${ANDROID_ABI}" in
  arm64-v8a)    TRIPLET="arm64-android" ;;
  armeabi-v7a)  TRIPLET="arm-neon-android" ;;
  x86_64)       TRIPLET="x64-android" ;;
  x86)          TRIPLET="x86-android" ;;
  *)
    echo "ERROR: Unsupported ABI '${ANDROID_ABI}'. Use: arm64-v8a, armeabi-v7a, x86_64, x86"
    exit 1
    ;;
esac

BUILD_DIR="${SCRIPT_DIR}/build-android-${ANDROID_ABI}"

echo "=== MSTelemetry vcpkg port test (Android cross-compile) ==="
echo "Repository root: ${REPO_ROOT}"
echo "ABI: ${ANDROID_ABI}"
echo "Triplet: ${TRIPLET}"

# Check prerequisites
if [ -z "${VCPKG_ROOT}" ]; then
  echo "ERROR: VCPKG_ROOT is not set. Please set it to your vcpkg installation directory."
  exit 1
fi

VCPKG_TOOLCHAIN="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "${VCPKG_TOOLCHAIN}" ]; then
  echo "ERROR: vcpkg toolchain not found at ${VCPKG_TOOLCHAIN}"
  exit 1
fi

# Find NDK
if [ -z "${ANDROID_NDK_HOME}" ]; then
  # Try common locations
  if [ -n "${ANDROID_HOME}" ] && [ -d "${ANDROID_HOME}/ndk" ]; then
    ANDROID_NDK_HOME=$(ls -d "${ANDROID_HOME}/ndk/"* 2>/dev/null | sort -t. -k1,1n -k2,2n -k3,3n | tail -1)
  elif [ -n "${ANDROID_SDK_ROOT}" ] && [ -d "${ANDROID_SDK_ROOT}/ndk" ]; then
    ANDROID_NDK_HOME=$(ls -d "${ANDROID_SDK_ROOT}/ndk/"* 2>/dev/null | sort -t. -k1,1n -k2,2n -k3,3n | tail -1)
  fi
fi

if [ -z "${ANDROID_NDK_HOME}" ] || [ ! -d "${ANDROID_NDK_HOME}" ]; then
  echo "ERROR: Android NDK not found. Set ANDROID_NDK_HOME to your NDK path."
  echo "       e.g., export ANDROID_NDK_HOME=\$ANDROID_HOME/ndk/26.1.10909125"
  exit 1
fi
echo "NDK: ${ANDROID_NDK_HOME}"

# Clean previous build
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

echo ""
echo "--- Step 1: Configure (vcpkg installs deps automatically) ---"
cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}/consumer" \
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_TOOLCHAIN}" \
  -DVCPKG_TARGET_TRIPLET="${TRIPLET}" \
  -DVCPKG_OVERLAY_PORTS="${OVERLAY_PORTS}" \
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="${ANDROID_ABI}" \
  -DANDROID_PLATFORM=android-28 \
  -DCMAKE_BUILD_TYPE=Release

echo ""
echo "--- Step 2: Build test consumer for Android ---"
cmake --build "${BUILD_DIR}/consumer" --config Release

echo ""
echo "--- Step 3: Verify output ---"
BINARY=$(find "${BUILD_DIR}/consumer" -name "vcpkg_test" -type f 2>/dev/null | head -1)
if [ -n "${BINARY}" ]; then
  echo "[PASS] Android binary produced: ${BINARY}"
  file "${BINARY}"
  echo ""
  echo "NOTE: Cross-compiled binary cannot be executed on the host."
  echo "      Deploy to device or emulator via adb for runtime verification:"
  echo "        adb push ${BINARY} /data/local/tmp/"
  echo "        adb shell /data/local/tmp/vcpkg_test"
else
  echo "[FAIL] Android binary not found"
  exit 1
fi

echo ""
echo "=== Android vcpkg port cross-compile test PASSED ==="
