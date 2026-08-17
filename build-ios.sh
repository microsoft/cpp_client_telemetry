#!/bin/bash

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"
. "$DIR/tools/build-common.sh"

#  The expected iOS build invocation is:
#    build-ios.sh [clean] [release|debug] ${ARCH} ${PLATFORM}
#  where
#    ARCH = arm64|arm64e|x86_64
#    PLATFORM = iphoneos|iphonesimulator|maccatalyst|xros|xrsimulator

if [ "$1" == "clean" ]; then
  matsdk_clean_build_outputs "build-ios.sh"
  shift
fi

BUILD_TYPE="Debug"
if [ "$1" == "release" ]; then
  BUILD_TYPE="Release"
  shift
elif [ "$1" == "debug" ]; then
  BUILD_TYPE="Debug"
  shift
fi

# Set Architecture: arm64, arm64e or x86_64
APPLE_ARCH=$(/usr/bin/uname -m)
if [ "$1" == "arm64" ]; then
  APPLE_ARCH="arm64"
  shift
elif [ "$1" == "arm64e" ]; then
  APPLE_ARCH="arm64e"
  shift
elif [ "$1" == "x86_64" ]; then
  APPLE_ARCH="x86_64"
  shift
fi

# the last param is expected to specify the platform name: iphoneos|iphonesimulator|maccatalyst|xros|xrsimulator
# so if it is non-empty and it is not "device", we take it as a valid platform name
# otherwise we fall back to old iOS logic which only supported iphoneos|iphonesimulator
APPLE_PLATFORM="iphonesimulator"
if [ -n "$1" ] && [ "$1" != "device" ]; then
  APPLE_PLATFORM="$1"
elif [ "$1" == "device" ]; then
  APPLE_PLATFORM="iphoneos"
fi

echo "architecture = $APPLE_ARCH, platform = $APPLE_PLATFORM, build type = $BUILD_TYPE"

DEPLOYMENT_TARGET=""

case "$APPLE_PLATFORM" in
  iphoneos|iphonesimulator)
    SYS_NAME="iOS"
    APPLE_SYSROOT="$APPLE_PLATFORM"
    DEPLOYMENT_TARGET="$CMAKE_OSX_DEPLOYMENT_TARGET"
    if [ -z "$DEPLOYMENT_TARGET" ]; then
      DEPLOYMENT_TARGET="13.0"
    fi
    ;;
  maccatalyst)
    SYS_NAME="iOS"
    APPLE_SYSROOT="macosx"
    DEPLOYMENT_TARGET="$MACCATALYST_DEPLOYMENT_TARGET"
    if [ -z "$DEPLOYMENT_TARGET" ]; then
      DEPLOYMENT_TARGET="14.0"
    fi
    ;;
  xros|xrsimulator)
    SYS_NAME="visionOS"
    APPLE_SYSROOT="$APPLE_PLATFORM"
    DEPLOYMENT_TARGET="$CMAKE_OSX_DEPLOYMENT_TARGET"
    if [ -z "$DEPLOYMENT_TARGET" ]; then
      DEPLOYMENT_TARGET="1.0"
    fi
    ;;
  *)
    echo "Unsupported Apple platform '$APPLE_PLATFORM'. Expected iphoneos, iphonesimulator, maccatalyst, xros, or xrsimulator." >&2
    exit 1
    ;;
esac

echo "deployment target = $DEPLOYMENT_TARGET"

# Install build tools and recent sqlite3
BUILD_TOOLS_MARKER=".buildtools"
matsdk_install_buildtools_once "$BUILD_TOOLS_MARKER" tools/setup-buildtools-apple.sh ios

matsdk_print_compiler_versions
matsdk_require_cmake_preset_support

CPACK_GENERATOR=TGZ
case "$APPLE_PLATFORM" in
  *simulator) PLATFORM_PRESET="matsdk-ios-simulator" ;;
  *)         PLATFORM_PRESET="matsdk-ios-device" ;;
esac
PRESET="${PLATFORM_PRESET}-$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"

cmake_args=(
  cmake --preset "$PRESET"
  "-DCMAKE_SYSTEM_NAME=$SYS_NAME"
  "-DCMAKE_OSX_SYSROOT=$APPLE_SYSROOT"
  "-DCMAKE_OSX_ARCHITECTURES=$APPLE_ARCH"
  "-DCMAKE_OSX_DEPLOYMENT_TARGET=$DEPLOYMENT_TARGET"
  "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
  "-DCPACK_GENERATOR=$CPACK_GENERATOR"
)
if [ "$APPLE_PLATFORM" == "maccatalyst" ]; then
  CMAKE_TARGET="${APPLE_ARCH}-apple-ios${DEPLOYMENT_TARGET}-macabi"
  IOS_SUPPORT_FRAMEWORKS="$(xcrun --sdk macosx --show-sdk-path)/System/iOSSupport/System/Library/Frameworks"
  cmake_args+=(
    "-DCMAKE_C_COMPILER_TARGET=$CMAKE_TARGET"
    "-DCMAKE_CXX_COMPILER_TARGET=$CMAKE_TARGET"
    "-DCMAKE_OSX_DEPLOYMENT_TARGET="
    "-DMATSDK_PLATFORM_IOS=ON"
    "-DCMAKE_C_FLAGS=-iframework ${IOS_SUPPORT_FRAMEWORKS}"
    "-DCMAKE_CXX_FLAGS=-iframework ${IOS_SUPPORT_FRAMEWORKS}"
  )
fi
matsdk_append_cmake_opts_to_cmake_args
matsdk_run_logged_command "${cmake_args[@]}"

cmake --build --preset "$PRESET"
if [ "${MATTELEMETRY_SKIP_PACKAGE:-}" != "1" ]; then
  cmake --build --preset "$PRESET" --target package
fi
