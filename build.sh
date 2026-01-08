#!/bin/bash

usage()
{
  echo "Usage: build.sh [clean] [arm64|x86_64|universal] [CUSTOM_CMAKE_CXX_FLAGS=x] [noroot] [release] [-h|-?] [-l (static|shared)] [-D CMAKE_OPTION] [-v]"
  echo "                                                                                         "
  echo "options:                                                                                 "
  echo "                                                                                         "
  echo "Positional options (1st three arguments):                                                "
  echo "[clean]                  - perform clean build                                           "
  echo "[arm64|x86_64|universal] - Apple platform build type. Not applicable to other OS.        "
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
  echo "MACOSX_DEPLOYMENT_TARGET - optional parameter for setting macosx deployment target       "
  echo "Plus any other environment variables respected by CMake build system.                    "
  exit 0
}

export PATH=/usr/local/bin:$PATH

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Current directory: $DIR"
cd $DIR

export NOROOT=$NOROOT

PARAM1="$1"
PARAM2="$2"
PARAM3="$3"

if [ "$PARAM1" == "clean" ]; then
  echo "Cleaning previous build artifacts"
  rm -f CMakeCache.txt *.cmake
  rm -rf out
  rm -rf .buildtools
  # make clean
  shift
fi

if [ "$PARAM1" == "noroot" ] || [ "$PARAM2" == "noroot" ] || [ "$PARAM3" == "noroot" ]; then
  export NOROOT=true
  echo "NOROOT = true"
  shift
fi

if [ "$PARAM1" == "release" ] || [ "$PARAM2" == "release" ] || [ "$PARAM3" == "release" ]; then
  BUILD_TYPE="Release"
  echo "BUILD_TYPE = Release"
  shift
elif [ "$PARAM1" == "debug" ] || [ "$PARAM2" == "debug" ] || [ "$PARAM3" == "debug" ]; then
  BUILD_TYPE="Debug"
  echo "BUILD_TYPE = Debug"
  shift
else
  BUILD_TYPE="Debug"
  echo "Assuming default BUILD_TYPE = Debug"
fi

if [ "$PARAM1" == "arm64" ] || [ "$PARAM2" == "arm64" ] || [ "$PARAM3" == "arm64" ]; then
  MAC_ARCH="arm64"
  echo "MAC_ARCH = arm64"
  shift
elif [ "$PARAM1" == "universal" ] || [ "$PARAM2" == "universal" ] || [ "$PARAM3" == "universal" ]; then
  MAC_ARCH="universal"
  echo "MAC_ARCH = universal"
  shift
elif [ "$PARAM1" == "x86_64" ] || [ "$PARAM2" == "x86_64" ] || [ "$PARAM3" == "x86_64" ]; then
  MAC_ARCH="x86_64"
  echo "MAC_ARCH = x86_64"
  shift
else
  MAC_ARCH=$(/usr/bin/uname -m)
  echo "Using current machine MAC_ARCH = $MAC_ARCH"
fi

CUSTOM_CMAKE_CXX_FLAG=""
if [[ $PARAM1 == CUSTOM_BUILD_FLAGS* ]] || [[ $PARAM2 == CUSTOM_BUILD_FLAGS* ]] || [[ $PARAM3 == CUSTOM_BUILD_FLAGS* ]]; then
  if [[ $PARAM1 == CUSTOM_BUILD_FLAGS* ]]; then
  CUSTOM_CMAKE_CXX_FLAG="\"${PARAM1:19:999}\""
  elif [[ $PARAM2 == CUSTOM_BUILD_FLAGS* ]]; then
  CUSTOM_CMAKE_CXX_FLAG="\"${PARAM2:19:999}\""
  elif [[ $PARAM3 == CUSTOM_BUILD_FLAGS* ]]; then
  CUSTOM_CMAKE_CXX_FLAG="\"${PARAM3:19:999}\""
  fi
  shift
  echo "custom build flags = $CUSTOM_CMAKE_CXX_FLAG"
fi

LINK_TYPE=
CMAKE_OPTS="${CMAKE_OPTS:--DBUILD_SHARED_LIBS=OFF}"
while getopts "h?vl:D:" opt; do
    case "$opt" in
    h|\?) usage
        ;;
    :)  echo "Invalid option: $OPTARG requires an argument" 1>&2
        exit 0
        ;;
    v)  verbose=1
        ;;
    D)  CMAKE_OPTS="${CMAKE_OPTS} -D${OPTARG}"
        ;;
    l)  LINK_TYPE=$OPTARG
        ;;
    esac
done
shift $((OPTIND -1))

echo "CMAKE_OPTS from caller: $CMAKE_OPTS"

if [ "$LINK_TYPE" == "shared" ]; then
  CMAKE_OPTS="${CMAKE_OPTS} -DBUILD_SHARED_LIBS=ON"
fi

# Set target MacOS minver
default_mac_os_target="11.10"
[ -z $MACOSX_DEPLOYMENT_TARGET ] && export MACOSX_DEPLOYMENT_TARGET=${default_mac_os_target}
echo "macosx deployment target="$MACOSX_DEPLOYMENT_TARGET

# Install build tools and recent sqlite3
FILE=.buildtools
OS_NAME=`uname -a`

if [ ! -f $FILE ]; then
  case "$OS_NAME" in
    *Darwin*) CMD="tools/setup-buildtools-apple.sh $MAC_ARCH" ;;
    *Linux*)  CMD="tools/setup-buildtools.sh" ;;
    *)        CMD=""; echo "WARNING: unsupported OS $OS_NAME, skipping build tools installation.." ;;
  esac

  [[ -n "$CMD" ]] && { [[ -z "$NOROOT" ]] && sudo $CMD || echo "No root: skipping build tools installation."; }
  echo > $FILE
fi

if [ -f /usr/bin/gcc ]; then
  echo "gcc   version: `gcc --version`"
fi

if [ -f /usr/bin/clang ]; then
  echo "clang version: `clang --version`"
fi

# Skip Version.hpp changes
# git update-index --skip-worktree lib/include/public/Version.hpp

#rm -rf out
mkdir -p out
cd out

# .tgz package
CMAKE_PACKAGE_TYPE=tgz

if [ -f /usr/bin/dpkg ]; then
  # .deb package
  export CMAKE_PACKAGE_TYPE=deb
elif [ -f /usr/bin/rpmbuild ]; then
  # .rpm package
  export CMAKE_PACKAGE_TYPE=rpm
fi

# Fail on error
set -e

# TODO: should this be improved to verify if the platform is Apple? Right now we unconditionally pass -DMAC_ARCH even if building for Windows or Linux.
cmake_cmd="cmake -DMAC_ARCH=$MAC_ARCH -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE -DCMAKE_CXX_FLAGS="${CUSTOM_CMAKE_CXX_FLAG}" $CMAKE_OPTS .."
echo $cmake_cmd
eval $cmake_cmd
# TODO: strip symbols to minimize (release-only)

# Build all
# TODO: what are the pros and cons of using 'make' vs 'cmake --build' ?
#make
cmake --build .

# Remove old package
rm -f *.deb *.rpm

# Build new package
case "$OS_NAME" in
  *Darwin*) sudo make package ;;
  *Linux*)  make package ;;
  *)        ;;
esac

# Install newly generated package
if [ -f /usr/bin/dpkg ]; then
  # Ubuntu / Debian / Raspbian 
  [[ -z "$NOROOT" ]] && sudo dpkg -i *.deb || echo "No root: skipping package deployment."
elif [ -f /usr/bin/rpmbuild ]; then
  # Redhat / Centos
  [[ -z "$NOROOT" ]] && sudo rpm -i --force -v *.rpm || echo "No root: skipping package deployment."
fi

# Install SDK headers and lib to /usr/local
#
## TODO: [MG] - fix this section for shared library
## strip --strip-unneeded out/lib/libmat.so
## strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag out/lib/libmat.so

if [ "$CMAKE_PACKAGE_TYPE" == "tgz" ]; then
  cd ..
  MATSDK_INSTALL_DIR="${MATSDK_INSTALL_DIR:-/usr/local}"
  echo "+-----------------------------------------------------------------------------------+"
  echo " This step may prompt for your sudo password to deploy SDK to $MATSDK_INSTALL_DIR  "
  echo "+-----------------------------------------------------------------------------------+"
  [[ -z "$NOROOT" ]] && sudo ./install.sh $MATSDK_INSTALL_DIR || echo "No root: skipping package deployment."
fi
