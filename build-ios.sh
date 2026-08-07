#!/bin/sh

#  The expected iOS build invocation is:
#    build-ios.sh [clean] [release|debug] ${ARCH} ${PLATFORM}
#  where
#    ARCH = arm64|arm64e|x86_64
#    PLATFORM = iphoneos|iphonesimulator|xros|xrsimulator

if [ "$1" == "clean" ]; then
  echo "build-ios.sh: cleaning previous build artifacts"
  rm -f CMakeCache.txt *.cmake
  rm -rf out
  rm -rf .buildtools
#  make clean
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

# the last param is expected to specify the platform name: iphoneos|iphonesimulator|xros|xrsimulator
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

if [ "$APPLE_PLATFORM" == "iphoneos" ] || [ "$APPLE_PLATFORM" == "iphonesimulator" ]; then
  SYS_NAME="iOS"
  DEPLOYMENT_TARGET="$CMAKE_OSX_DEPLOYMENT_TARGET"
  if [ -z "$DEPLOYMENT_TARGET" ]; then
    DEPLOYMENT_TARGET="12.0"
  fi
elif [ "$APPLE_PLATFORM" == "xros" ] || [ "$APPLE_PLATFORM" == "xrsimulator" ]; then
  SYS_NAME="visionOS"
  DEPLOYMENT_TARGET="$CMAKE_OSX_DEPLOYMENT_TARGET"
  if [ -z "$DEPLOYMENT_TARGET" ]; then
    DEPLOYMENT_TARGET="1.0"
  fi
fi

echo "deployment target = $DEPLOYMENT_TARGET"

# Install build tools and recent sqlite3
FILE=".buildtools"
if [ ! -f $FILE ]; then
  tools/setup-buildtools-apple.sh ios
  # Assume that the build tools have been successfully installed
  echo > $FILE
fi

if [ -f /usr/bin/gcc ]; then
  echo "gcc   version: `gcc --version`"
fi

if [ -f /usr/bin/clang ]; then
  echo "clang version: `clang --version`"
fi

mkdir -p out
cd out

CMAKE_PACKAGE_TYPE=tgz

cmake_cmd="cmake -DCMAKE_OSX_SYSROOT=$APPLE_PLATFORM -DCMAKE_SYSTEM_NAME=$SYS_NAME -DCMAKE_OSX_ARCHITECTURES=$APPLE_ARCH -DCMAKE_OSX_DEPLOYMENT_TARGET=$DEPLOYMENT_TARGET -DBUILD_IOS=YES -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE $CMAKE_OPTS .."
echo "${cmake_cmd}"
eval $cmake_cmd

make

make package
