#!/usr/bin/env bash
set -euo pipefail

# Build MAESDK Android AAR locally (debug+release) and collect symbols.
# Mirrors lib/modules/.github/workflows/build-android-aar.yml, simplified for local use.
#
# Usage:
#   tools/build-android-aar.sh
#   tools/build-android-aar.sh --install-ndk
#   tools/build-android-aar.sh --ndk-version 27.0.12077973 --output-dir /tmp/out
#
# Notes:
# - Requires Java 17 and Android SDK cmdline-tools (sdkmanager) if you use --install-ndk.
# - Requires ANDROID_SDK_ROOT (or ANDROID_HOME) when installing NDK.

ANDROID_NDK_VERSION_DEFAULT="27.0.12077973"
CMAKE_VERSION_DEFAULT="3.10.2.4988404"

INSTALL_NDK=0
CLEAN=0
NDK_VERSION="$ANDROID_NDK_VERSION_DEFAULT"
CMAKE_VERSION="$CMAKE_VERSION_DEFAULT"
OUTPUT_DIR=""

usage() {
  cat <<'EOF'
Build MAESDK Android AAR locally.

Options:
  --install-ndk              Install the pinned NDK + CMake via sdkmanager.
  --ndk-version <version>    Override NDK version (default: 27.0.12077973).
  --cmake-version <version>  Override CMake version (default: 3.10.2.4988404).
  --output-dir <path>        Output directory (default: <repo>/dist/android-aar).
  --clean                    Run a clean build (gradle clean).
  -h, --help                 Show help.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --install-ndk) INSTALL_NDK=1; shift ;;
    --clean) CLEAN=1; shift ;;
    --ndk-version) NDK_VERSION="${2:?}"; shift 2 ;;
    --cmake-version) CMAKE_VERSION="${2:?}"; shift 2 ;;
    --output-dir) OUTPUT_DIR="${2:?}"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown arg: $1" >&2; usage; exit 2 ;;
  esac
done

# Resolve repo root (works whether invoked from repo root or elsewhere).
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." >/dev/null 2>&1 && pwd)"

ANDROID_BUILD_DIR="${REPO_ROOT}/lib/android_build"
MAESDK_DIR="${ANDROID_BUILD_DIR}/maesdk"
VERSION_HEADER="${REPO_ROOT}/lib/include/public/Version.hpp"

if [[ ! -d "$ANDROID_BUILD_DIR" ]]; then
  echo "Expected Android build directory not found: $ANDROID_BUILD_DIR" >&2
  exit 1
fi
if [[ ! -f "$VERSION_HEADER" ]]; then
  echo "Expected version header not found: $VERSION_HEADER" >&2
  exit 1
fi

if [[ -z "$OUTPUT_DIR" ]]; then
  OUTPUT_DIR="${REPO_ROOT}/dist/android-aar"
fi

# Extract BUILD_VERSION_STR from Version.hpp (matches the CI workflow logic).
MAESDK_VERSION="$(sed -n 's/.*BUILD_VERSION_STR "\(.*\)"/\1/p' < "$VERSION_HEADER" | head -n 1)"
if [[ -z "$MAESDK_VERSION" ]]; then
  echo "Failed to parse BUILD_VERSION_STR from: $VERSION_HEADER" >&2
  exit 1
fi

if command -v git >/dev/null 2>&1 && git -C "$REPO_ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  SHORT_HASH="$(git -C "$REPO_ROOT" rev-parse --short HEAD)"
else
  SHORT_HASH="nogit"
fi

echo "==> Repo: $REPO_ROOT"
echo "==> Version: $MAESDK_VERSION"
echo "==> Short hash: $SHORT_HASH"
echo "==> Output: $OUTPUT_DIR"

if [[ $INSTALL_NDK -eq 1 ]]; then
  ANDROID_SDK_ROOT_EFFECTIVE="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
  if [[ -z "$ANDROID_SDK_ROOT_EFFECTIVE" ]]; then
    echo "ANDROID_SDK_ROOT (or ANDROID_HOME) must be set for --install-ndk" >&2
    exit 1
  fi

  SDKMANAGER="${ANDROID_SDK_ROOT_EFFECTIVE}/cmdline-tools/latest/bin/sdkmanager"
  if [[ ! -x "$SDKMANAGER" ]]; then
    echo "sdkmanager not found/executable at: $SDKMANAGER" >&2
    echo "Install Android SDK Command-line Tools (latest) and ensure ANDROID_SDK_ROOT is correct." >&2
    exit 1
  fi

  echo "==> Installing Android NDK $NDK_VERSION and CMake $CMAKE_VERSION (via sdkmanager)"
  # Use 'yes' to auto-accept licenses.
  yes | "$SDKMANAGER" --install "ndk-bundle" "cmake;${CMAKE_VERSION}" "ndk;${NDK_VERSION}" --sdk_root="${ANDROID_SDK_ROOT_EFFECTIVE}"
fi

# CI removes ~/.m2/settings.xml to avoid an Actions-specific issue.
# Locally, this can be legitimate user config, so we only remove it if it looks like a GitHub-provided file.
if [[ -f "$HOME/.m2/settings.xml" ]]; then
  if grep -q "github" "$HOME/.m2/settings.xml" 2>/dev/null; then
    echo "==> Removing ~/.m2/settings.xml (contains 'github', likely CI-provided)"
    rm -f "$HOME/.m2/settings.xml"
  else
    echo "==> Keeping ~/.m2/settings.xml (user config)"
  fi
fi

pushd "$ANDROID_BUILD_DIR" >/dev/null

if [[ $CLEAN -eq 1 ]]; then
  echo "==> Gradle clean"
  ./gradlew clean
fi

echo "==> Gradle maesdk:build"
./gradlew maesdk:build

popd >/dev/null

# Package artifacts similar to the CI workflow.
mkdir -p "$OUTPUT_DIR/symbols/debug" "$OUTPUT_DIR/symbols/release"

DEBUG_AAR_SRC="${MAESDK_DIR}/build/outputs/aar/maesdk-debug.aar"
RELEASE_AAR_SRC="${MAESDK_DIR}/build/outputs/aar/maesdk-release.aar"

# Some Gradle setups publish into a 'dist' folder; tolerate that layout as a fallback.
if [[ ! -f "$DEBUG_AAR_SRC" && -f "${MAESDK_DIR}/dist/outputs/aar/maesdk-debug.aar" ]]; then
  DEBUG_AAR_SRC="${MAESDK_DIR}/dist/outputs/aar/maesdk-debug.aar"
fi
if [[ ! -f "$RELEASE_AAR_SRC" && -f "${MAESDK_DIR}/dist/outputs/aar/maesdk-release.aar" ]]; then
  RELEASE_AAR_SRC="${MAESDK_DIR}/dist/outputs/aar/maesdk-release.aar"
fi

if [[ ! -f "$DEBUG_AAR_SRC" ]]; then
  echo "Missing debug AAR: $DEBUG_AAR_SRC" >&2
  exit 1
fi
if [[ ! -f "$RELEASE_AAR_SRC" ]]; then
  echo "Missing release AAR: $RELEASE_AAR_SRC" >&2
  exit 1
fi

DEBUG_AAR_DST="${OUTPUT_DIR}/maesdk-debug-v${MAESDK_VERSION}-${SHORT_HASH}.aar"
RELEASE_AAR_DST="${OUTPUT_DIR}/maesdk-release-v${MAESDK_VERSION}-${SHORT_HASH}.aar"

cp -f "$DEBUG_AAR_SRC" "$DEBUG_AAR_DST"
cp -f "$RELEASE_AAR_SRC" "$RELEASE_AAR_DST"

# Copy symbols if present.
# CI uses the older CMake intermediates path; locally, newer Android Gradle Plugin versions often use 'intermediates/cxx'.

copy_symbols_dir() {
  local src_dir="$1"
  local dst_dir="$2"
  if [[ -d "$src_dir" ]]; then
    cp -R "$src_dir"/* "$dst_dir" 2>/dev/null || true
    return 0
  fi
  return 1
}

DEBUG_SYMS_CMAKE="${MAESDK_DIR}/build/intermediates/cmake/debug/obj"
RELEASE_SYMS_CMAKE="${MAESDK_DIR}/build/intermediates/cmake/release/obj"

DEBUG_SYMS_CXX_BASE="${MAESDK_DIR}/build/intermediates/cxx/Debug"
RELEASE_SYMS_CXX_BASE="${MAESDK_DIR}/build/intermediates/cxx/RelWithDebInfo"

if ! copy_symbols_dir "$DEBUG_SYMS_CMAKE" "$OUTPUT_DIR/symbols/debug"; then
  # Newer layout: .../intermediates/cxx/Debug/<hash>/obj
  if [[ -d "$DEBUG_SYMS_CXX_BASE" ]]; then
    found=0
    while IFS= read -r -d '' obj_dir; do
      copy_symbols_dir "$obj_dir" "$OUTPUT_DIR/symbols/debug" || true
      found=1
    done < <(find "$DEBUG_SYMS_CXX_BASE" -maxdepth 2 -type d -name obj -print0 2>/dev/null || true)
    [[ $found -eq 1 ]] || echo "==> Debug symbols not found under: $DEBUG_SYMS_CXX_BASE"
  else
    echo "==> Debug symbols dir not found (skipping): $DEBUG_SYMS_CMAKE"
  fi
fi

if ! copy_symbols_dir "$RELEASE_SYMS_CMAKE" "$OUTPUT_DIR/symbols/release"; then
  if [[ -d "$RELEASE_SYMS_CXX_BASE" ]]; then
    found=0
    while IFS= read -r -d '' obj_dir; do
      copy_symbols_dir "$obj_dir" "$OUTPUT_DIR/symbols/release" || true
      found=1
    done < <(find "$RELEASE_SYMS_CXX_BASE" -maxdepth 2 -type d -name obj -print0 2>/dev/null || true)
    [[ $found -eq 1 ]] || echo "==> Release symbols not found under: $RELEASE_SYMS_CXX_BASE"
  else
    echo "==> Release symbols dir not found (skipping): $RELEASE_SYMS_CMAKE"
  fi
fi

echo "==> Done"
echo "    $DEBUG_AAR_DST"
echo "    $RELEASE_AAR_DST"
echo "    $OUTPUT_DIR/symbols/"
