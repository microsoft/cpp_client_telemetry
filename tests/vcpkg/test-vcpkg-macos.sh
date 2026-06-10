#!/bin/bash
# Test script: Verify mstelemetry vcpkg port on macOS
# Usage: ./tests/vcpkg/test-vcpkg-macos.sh
# Prerequisites: VCPKG_ROOT set, Xcode command line tools, cmake
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-macos"
OVERLAY_PORTS="${REPO_ROOT}/tools/ports"

echo "=== MSTelemetry vcpkg port test (macOS) ==="
echo "Repository root: ${REPO_ROOT}"

# Check prerequisites
if [ -z "${VCPKG_ROOT}" ]; then
  echo "ERROR: VCPKG_ROOT is not set. Please set it to your vcpkg installation directory."
  exit 1
fi

if [ "$(uname)" != "Darwin" ]; then
  echo "ERROR: This script must be run on macOS."
  exit 1
fi

VCPKG_TOOLCHAIN="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "${VCPKG_TOOLCHAIN}" ]; then
  echo "ERROR: vcpkg toolchain not found at ${VCPKG_TOOLCHAIN}"
  exit 1
fi
MAC_ARCH="$(uname -m)"

if [ "${MAC_ARCH}" = "arm64" ]; then
  TRIPLET="arm64-osx"
else
  TRIPLET="x64-osx"
fi

echo "Architecture: ${MAC_ARCH}"
echo "Triplet:      ${TRIPLET}"

# Clean previous build
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

echo ""
echo "--- Step 1: Configure (vcpkg installs deps automatically) ---"
cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}/consumer" \
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_TOOLCHAIN}" \
  -DVCPKG_TARGET_TRIPLET="${TRIPLET}" \
  -DVCPKG_OVERLAY_PORTS="${OVERLAY_PORTS}" \
  -DCMAKE_BUILD_TYPE=Release

echo ""
echo "--- Step 2: Build test consumer ---"
cmake --build "${BUILD_DIR}/consumer" --config Release

echo ""
echo "--- Step 3: Run test ---"
"${BUILD_DIR}/consumer/vcpkg_test"

echo ""
echo "=== macOS vcpkg port test PASSED ==="
