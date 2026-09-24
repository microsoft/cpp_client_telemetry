#!/bin/bash
# Test script: Verify cpp-client-telemetry vcpkg port for Android (cross-compile only)
# Usage: ./tests/vcpkg/test-vcpkg-android.sh [ABI] [API_LEVEL]
#   ABI: arm64-v8a (default), armeabi-v7a, x86_64, x86
#   API_LEVEL: 23 (default), 28, or another level with a matching overlay triplet
# Prerequisites: VCPKG_ROOT set, ANDROID_NDK_HOME set, cmake, ninja
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OVERLAY_PORTS="${REPO_ROOT}/tools/ports"

# Build the working tree under review (not a pinned release) so this test
# validates the actual SDK source together with the port manifest/portfile.
export MATSDK_VCPKG_SOURCE_DIR="${REPO_ROOT}"

# Android ABI/API (defaults match the repo's Android minSdk)
ANDROID_ABI="${1:-arm64-v8a}"
ANDROID_API="${2:-23}"

# Map ABI to vcpkg triplet
case "${ANDROID_ABI}" in
  arm64-v8a)    BASE_TRIPLET="arm64-android" ;;
  armeabi-v7a)  BASE_TRIPLET="arm-neon-android" ;;
  x86_64)       BASE_TRIPLET="x64-android" ;;
  x86)          BASE_TRIPLET="x86-android" ;;
  *)
    echo "ERROR: Unsupported ABI '${ANDROID_ABI}'. Use: arm64-v8a, armeabi-v7a, x86_64, x86"
    exit 1
    ;;
esac

if [ "${ANDROID_API}" = "28" ]; then
  TRIPLET="${BASE_TRIPLET}"
  OVERLAY_TRIPLETS_ARGS=()
else
  TRIPLET="${BASE_TRIPLET}-api${ANDROID_API}"
  OVERLAY_TRIPLETS="${SCRIPT_DIR}/triplets"
  if [ ! -f "${OVERLAY_TRIPLETS}/${TRIPLET}.cmake" ]; then
    echo "ERROR: Android API ${ANDROID_API} requires overlay triplet ${TRIPLET}."
    echo "       Add ${OVERLAY_TRIPLETS}/${TRIPLET}.cmake or use API 23/28."
    exit 1
  fi
  OVERLAY_TRIPLETS_ARGS=(-DVCPKG_OVERLAY_TRIPLETS="${OVERLAY_TRIPLETS}")
fi

BUILD_DIR="${SCRIPT_DIR}/build-android-${ANDROID_ABI}-api${ANDROID_API}"

echo "=== MSTelemetry vcpkg port test (Android cross-compile) ==="
echo "Repository root: ${REPO_ROOT}"
echo "ABI: ${ANDROID_ABI}"
echo "Android API: ${ANDROID_API}"
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

if ! command -v ninja >/dev/null 2>&1; then
  echo "ERROR: ninja is required for Android vcpkg tests."
  echo "       Using Ninja avoids host IDE generators selecting a different Android NDK."
  exit 1
fi

# Clean previous build
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

echo ""
echo "--- Step 1: Configure (vcpkg installs deps automatically) ---"
cmake -G Ninja -S "${SCRIPT_DIR}" -B "${BUILD_DIR}/consumer" \
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_TOOLCHAIN}" \
  -DVCPKG_TARGET_TRIPLET="${TRIPLET}" \
  -DVCPKG_OVERLAY_PORTS="${OVERLAY_PORTS}" \
  "${OVERLAY_TRIPLETS_ARGS[@]}" \
  -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="${ANDROID_ABI}" \
  -DANDROID_PLATFORM=android-${ANDROID_API} \
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
