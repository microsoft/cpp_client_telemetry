#!/bin/sh

if [ "$1" == "release" ] || [ "$2" == "release" ]; then
BUILD_TYPE="Release"
else
BUILD_TYPE="Debug"
fi

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

cmake -DBUILD_IOS=YES -DBUILD_SIMULATOR=NO -DBUILD_UNIT_TESTS=NO -DBUILD_FUNC_TESTS=NO -DCMAKE_BUILD_TYPE=$BUILD_TYPE .
make

CMAKE_PACKAGE_TYPE=tgz
make package
