#!/bin/sh

if [ "$1" == "clean" ]; then
 rm -f CMakeCache.txt *.cmake
 rm -rf out
 rm -rf .buildtools
# make clean
fi

if [ "$1" == "release" ] || [ "$2" == "release" ]; then
BUILD_TYPE="Release"
else
BUILD_TYPE="Debug"
fi

# Set Architecture: arm64, arm64e or x86_64
if [ "$2" == "arm64" ] || [ "$3" == "arm64" ]; then
IOS_ARCH="arm64"
elif [ "$2" == "arm64e" ] || [ "$3" == "arm64e" ]; then
IOS_ARCH="arm64e"
else
IOS_ARCH="x86_64"
fi

# Set Platform: device or simulator
if [ "$2" == "device" ] || [ "$3" == "device" ] || [ "$4" == "device" ]; then
IOS_PLAT="iphoneos"
else
IOS_PLAT="iphonesimulator"
fi

# Set target iOS minver
IOS_DEPLOYMENT_TARGET=10.0
FORCE_RESET_DEPLOYMENT_TARGET=YES
if [[ $3 == IOS_DEPLOYMENT_TARGET* ]] || [[ $4 == IOS_DEPLOYMENT_TARGET* ]] || [[ $5 == IOS_DEPLOYMENT_TARGET* ]]; then
  if [[ $3 == IOS_DEPLOYMENT_TARGET* ]]; then
  IOS_DEPLOYMENT_TARGET=${3:22:999}
  elif [[ $4 == IOS_DEPLOYMENT_TARGET* ]]; then
  IOS_DEPLOYMENT_TARGET=${4:22:999}
  elif [[ $5 == IOS_DEPLOYMENT_TARGET* ]]; then
  IOS_DEPLOYMENT_TARGET=${5:22:999}
  fi
  FORCE_RESET_DEPLOYMENT_TARGET=N0
fi
echo "iOS deployment target="$IOS_DEPLOYMENT_TARGET
echo "force reset deployment target="$FORCE_RESET_DEPLOYMENT_TARGET

# Install build tools and recent sqlite3
FILE=.buildtools
OS_NAME=`uname -a`
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

cmake_cmd="cmake -DCMAKE_OSX_SYSROOT=$IOS_PLAT -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=$IOS_DEPLOYMENT_TARGET -DBUILD_IOS=YES -DIOS_ARCH=$IOS_ARCH -DIOS_PLAT=$IOS_PLAT -DIOS_DEPLOYMENT_TARGET=$IOS_DEPLOYMENT_TARGET -DBUILD_UNIT_TESTS=YES -DBUILD_FUNC_TESTS=YES -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE -DFORCE_RESET_DEPLOYMENT_TARGET=$FORCE_RESET_DEPLOYMENT_TARGET $CMAKE_OPTS .."
echo $cmake_cmd
eval $cmake_cmd

make

make package
