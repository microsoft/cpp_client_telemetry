#!/bin/bash

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"
. "$DIR/tools/build-common.sh"

#  The expected iOS build invocation is:
#    build-ios.sh [clean] [release|debug] ${ARCH} ${PLATFORM}
#  where
#    ARCH = arm64|arm64e|x86_64
#    PLATFORM = iphoneos|iphonesimulator|xros|xrsimulator

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
IOS_ARCH=$(/usr/bin/uname -m)
if [ "$1" == "arm64" ]; then
  IOS_ARCH="arm64"
  shift
elif [ "$1" == "arm64e" ]; then
  IOS_ARCH="arm64e"
  shift
elif [ "$1" == "x86_64" ]; then
  IOS_ARCH="x86_64"
  shift
fi

# the last param is expected to specify the platform name: iphoneos|iphonesimulator|xros|xrsimulator
# so if it is non-empty and it is not "device", we take it as a valid platform name
# otherwise we fall back to old iOS logic which only supported iphoneos|iphonesimulator
IOS_PLAT="iphonesimulator"
if [ -n "$1" ] && [ "$1" != "device" ]; then
  IOS_PLAT="$1"
elif [ "$1" == "device" ]; then
  IOS_PLAT="iphoneos"
fi

echo "IOS_ARCH = $IOS_ARCH, IOS_PLAT = $IOS_PLAT, BUILD_TYPE = $BUILD_TYPE"

DEPLOYMENT_TARGET=""

if [ "$IOS_PLAT" == "iphoneos" ] || [ "$IOS_PLAT" == "iphonesimulator" ]; then
  SYS_NAME="iOS"
  DEPLOYMENT_TARGET="$IOS_DEPLOYMENT_TARGET"
  if [ -z "$DEPLOYMENT_TARGET" ]; then
    DEPLOYMENT_TARGET="12.0"
  fi
elif [ "$IOS_PLAT" == "xros" ] || [ "$IOS_PLAT" == "xrsimulator" ]; then
  SYS_NAME="visionOS"
  DEPLOYMENT_TARGET="$XROS_DEPLOYMENT_TARGET"
  if [ -z "$DEPLOYMENT_TARGET" ]; then
    DEPLOYMENT_TARGET="1.0"
  fi
fi

echo "deployment target = $DEPLOYMENT_TARGET"

# Install build tools and recent sqlite3
BUILD_TOOLS_MARKER=".buildtools"
matsdk_install_buildtools_once "$BUILD_TOOLS_MARKER" tools/setup-buildtools-apple.sh ios

matsdk_print_compiler_versions
matsdk_require_cmake_preset_support

CPACK_GENERATOR=TGZ
case "$IOS_PLAT" in
  *simulator) PLATFORM_PRESET="matsdk-ios-simulator" ;;
  *)         PLATFORM_PRESET="matsdk-ios-device" ;;
esac
PRESET="${PLATFORM_PRESET}-$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"

cmake_args=(
  cmake --preset "$PRESET"
  "-DCMAKE_SYSTEM_NAME=$SYS_NAME"
  "-DCMAKE_OSX_SYSROOT=$IOS_PLAT"
  "-DCMAKE_OSX_ARCHITECTURES=$IOS_ARCH"
  "-DCMAKE_OSX_DEPLOYMENT_TARGET=$DEPLOYMENT_TARGET"
  "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
  "-DCPACK_GENERATOR=$CPACK_GENERATOR"
)
matsdk_append_cmake_opts_to_cmake_args
matsdk_run_logged_command "${cmake_args[@]}"

matsdk_build_and_package_preset "$PRESET"
