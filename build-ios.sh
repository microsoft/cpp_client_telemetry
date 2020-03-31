#!/bin/sh

if [ "$1" == "release" ]; then
BUILD_TYPE="Release"
else
BUILD_TYPE="Debug"
fi

if [ "$2" == "arm64" ]; then
IOS_ARCH="arm64"
elif [ "$2" == "arm64e" ]; then
IOS_ARCH="arm64e"
elif [ "$2" == "x86_64" ] || "$2" == "simulator" ]; then
IOS_ARCH="x86_64"
else
IOS_ARCH="x86_64"
fi

# Set target iOS minver
IOS_DEPLOYMENT_TARGET=10.10

# Install build tools and recent sqlite3
FILE=.buildtools
OS_NAME=`uname -a`
if [ ! -f $FILE ]; then
  tools/setup-buildtools-mac.sh
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

cmake -DBUILD_IOS=YES -DIOS_ARCH=$IOS_ARCH -DIOS_DEPLOYMENT_TARGET=$IOS_DEPLOYMENT_TARGET -DBUILD_UNIT_TESTS=YES -DBUILD_FUNC_TESTS=YES -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE ..
make

make package
