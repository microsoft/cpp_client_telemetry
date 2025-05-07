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

FORCE_RESET_DEPLOYMENT_TARGET=NO
DEPLOYMENT_TARGET=""

if [ "$IOS_PLAT" == "iphoneos" ] || [ "$IOS_PLAT" == "iphonesimulator" ]; then
  SYS_NAME="iOS"
  DEPLOYMENT_TARGET="$IOS_DEPLOYMENT_TARGET"
  if [ -z "$DEPLOYMENT_TARGET" ]; then
    DEPLOYMENT_TARGET="10.0"
    FORCE_RESET_DEPLOYMENT_TARGET=YES
  fi
elif [ "$IOS_PLAT" == "xros" ] || [ "$IOS_PLAT" == "xrsimulator" ]; then
  SYS_NAME="visionOS"
  DEPLOYMENT_TARGET="$XROS_DEPLOYMENT_TARGET"
  if [ -z "$DEPLOYMENT_TARGET" ]; then
    DEPLOYMENT_TARGET="1.0"
    FORCE_RESET_DEPLOYMENT_TARGET=YES
  fi
fi

echo "deployment target = $DEPLOYMENT_TARGET"
echo "force reset deployment target = $FORCE_RESET_DEPLOYMENT_TARGET"

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

cmake_cmd="cmake -DCMAKE_OSX_SYSROOT=$IOS_PLAT -DCMAKE_SYSTEM_NAME=$SYS_NAME -DCMAKE_IOS_ARCH_ABI=$IOS_ARCH -DCMAKE_OSX_DEPLOYMENT_TARGET=$DEPLOYMENT_TARGET -DCMAKE_C_COMPILER=$IOS_PLAT -DCMAKE_CXX_COMPILER=$IOS_PLAT -DBUILD_IOS=YES -DIOS_ARCH=$IOS_ARCH -DIOS_PLAT=$IOS_PLAT -DIOS_DEPLOYMENT_TARGET=$DEPLOYMENT_TARGET -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE -DFORCE_RESET_DEPLOYMENT_TARGET=$FORCE_RESET_DEPLOYMENT_TARGET $CMAKE_OPTS .."
echo "${cmake_cmd}"
eval $cmake_cmd

make

make package
