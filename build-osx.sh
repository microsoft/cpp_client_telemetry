#!/bin/bash

export PATH=/usr/local/bin:$PATH

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Current directory: $DIR"
cd $DIR

export NOROOT=$NOROOT

if [ "$1" == "clean" ]; then
  rm -f CMakeCache.txt *.cmake
  rm -rf out
  rm -rf .buildtools
  # make clean
fi

if [ "$1" == "noroot" ] || [ "$2" == "noroot" ] || [ "$3" == "noroot" ]; then
  export NOROOT=true
fi

if [ "$1" == "release" ] || [ "$2" == "release" ] || [ "$3" == "release" ]; then
  BUILD_TYPE="Release"
else
  BUILD_TYPE="Debug"
fi

if [ "$1" == "arm64" ] || [ "$2" == "arm64" ] || [ "$3" == "arm64" ]; then
  MAC_ARCH="arm64"
elif [ "$1" == "universal" ] || [ "$2" == "universal" ] || [ "$3" == "universal" ]; then
  MAC_ARCH="universal"
else
  MAC_ARCH="x86_64"
fi

CUSTOM_CMAKE_CXX_FLAG=""
if [[ $1 == CUSTOM_BUILD_FLAGS* ]] || [[ $2 == CUSTOM_BUILD_FLAGS* ]] || [[ $3 == CUSTOM_BUILD_FLAGS* ]]; then
  if [[ $1 == CUSTOM_BUILD_FLAGS* ]]; then
  CUSTOM_CMAKE_CXX_FLAG="\"${1:19:999}\""
  elif [[ $2 == CUSTOM_BUILD_FLAGS* ]]; then
  CUSTOM_CMAKE_CXX_FLAG="\"${2:19:999}\""
  elif [[ $3 == CUSTOM_BUILD_FLAGS* ]]; then
  CUSTOM_CMAKE_CXX_FLAG="\"${3:19:999}\""
  fi
  echo "custom build flags="$CUSTOM_CMAKE_CXX_FLAG
fi

LINK_TYPE=
CMAKE_OPTS="${CMAKE_OPTS:-DBUILD_SHARED_LIBS=OFF}"
while getopts "h?vl:D:" opt; do
    case "$opt" in
    h|\?)
        echo "Usage: build.sh [clean] [arm64|universal] [CUSTOM_CMAKE_CXX_FLAGS=x] [noroot] [release] [-h|-?] [-l (static|shared)] [-D CMAKE_OPTION] [-v]"
        echo "                                                                                         "
        echo "options:                                                                                 "
        echo "                                                                                         "
        echo "Positional options (1st three arguments):                                                "
        echo "[clean]                  - perform clean build                                           "
        echo "[arm64|universal]        - Apple platform build type. Not applicable to other OS.        "
        echo "[CUSTOM_CMAKE_CXX_FLAGS] - custom CXX compiler flags                                     "
        echo "[noroot]                 - custom CXX compiler flags                                     "
        echo "[release]                - build for Release                                             "
        echo "                                                                                         "
        echo "Additional parameters:                                                                   "
        echo " -h | -?                 - this help.                                                    "
        echo " -l [static|shared]      - build static (default) or shared library.                     "
        echo " -D [CMAKE_OPTION]       - additional options to pass to cmake. Could be multiple.       "
        echo " -v                      - increase build verbosity (reserved for future use)            "
        echo "                                                                                         "
        echo "Environment variables:                                                                   "
        echo "CMAKE_OPTS               - any additional cmake options.                                 "
        echo "GIT_PULL_TOKEN           - authorization token for Microsoft-proprietary modules.        "
        echo "Plus any other environment variables respected by CMake build system.                    "
        exit 0
        ;;
    :)  echo "Invalid option: $OPTARG requires an argument" 1>&2
        exit 0
        ;;
    v)  verbose=1
        ;;
    D)  CMAKE_OPTS="$CMAKE_OPTS -D$OPTARG"
        ;;
    l)  LINK_TYPE=$OPTARG
        ;;
    esac
done
shift $((OPTIND -1))

if [ "$LINK_TYPE" == "shared" ]; then
  CMAKE_OPTS="$CMAKE_OPTS -DBUILD_SHARED_LIBS=ON"
fi

# Set target MacOS minver
export MACOSX_DEPLOYMENT_TARGET=10.10

# Install build tools and recent sqlite3
FILE=.buildtools
OS_NAME=`uname -a`
if [ ! -f $FILE ]; then
  tools/setup-buildtools-apple.sh
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

cmake -DMAC_ARCH=$MAC_ARCH -DBUILD_UNIT_TESTS=YES -DBUILD_FUNC_TESTS=YES -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE -DCMAKE_CXX_FLAGS="${CUSTOM_CMAKE_CXX_FLAG}" $CMAKE_OPTS ..

make

make package