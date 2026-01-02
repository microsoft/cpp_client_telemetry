#!/bin/bash

usage()
{
  echo "Usage: build.sh [clean] [arm64|x86_64|universal] [CUSTOM_BUILD_FLAGS=x] [noroot] [release|debug] [-h|-?] [-l (static|shared)] [-D CMAKE_OPTION] [-v]"
  echo "                                                                                         "
  echo "options:                                                                                 "
  echo "                                                                                         "
  echo "Positional options:                                                                          "
  echo "[clean]                  - perform clean build                                           "
  echo "[arm64|x86_64|universal] - Apple platform build type. Not applicable to other OS.        "
  echo "[CUSTOM_BUILD_FLAGS]     - custom CXX compiler flags                                     "
  echo "[noroot]                 - custom CXX compiler flags                                     "
  echo "[release|debug]          - Specify build type (defaults to Debug)                        "
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

# Evaluate arguments that are not switches
while [[ $# -gt 0 ]]; do
  ARG="$1"
  case "$ARG" in
    clean|noroot|release|debug|arm64|x86_64|universal|CUSTOM_BUILD_FLAGS*)
      case "$ARG" in
        clean)
          CLEAN=true
          echo "CLEAN = true"
          ;;
        noroot)
          export NOROOT=true
          echo "NOROOT = true"
          ;;
        release|debug)
          if [[ -n "$BUILD_TYPE" ]]; then
              echo "Error: BUILD_TYPE is already set to '$BUILD_TYPE'. Cannot overwrite with $ARG." 1>&2
              exit 1
          elif [[ "$ARG" == "release" ]]; then
              BUILD_TYPE="Release"
          elif [[ "$ARG" == "debug" ]]; then
              BUILD_TYPE="Debug"
          fi
          echo "BUILD_TYPE = $BUILD_TYPE"
          ;;
        arm64|x86_64|universal)
          if [[ -n "$MAC_ARCH" ]]; then
              echo "Error: MAC_ARCH is already set to '$MAC_ARCH'. Cannot overwrite with $ARG." 1>&2
              exit 1
          else
              MAC_ARCH="$ARG"
          fi
          echo "MAC_ARCH = $MAC_ARCH"
          ;;
        CUSTOM_BUILD_FLAGS*)
          CUSTOM_CMAKE_CXX_FLAG="\"${ARG:19:999}\""
          echo "custom compiler flags = $CUSTOM_CMAKE_CXX_FLAG"
          ;;
        *)
          echo "Error: case not added: $ARG" 1>&2
          exit 1
          ;;
      esac
      shift
      ;;
    *)
      break
      ;;
  esac
done

if [[ -z "$BUILD_TYPE" ]]; then
  BUILD_TYPE="Debug"
  echo "Assuming default BUILD_TYPE = Debug"
fi

if [[ -z "$MAC_ARCH" ]]; then
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

# Detect args accidentally passed after the last switch argument
shift $((OPTIND -1))
if [[ $# -gt 0 ]]; then
    echo "Error: There are arguments remaining after parsing all positional arguments and switches: $@" 1>&2
    exit 1
fi

if [[ "$CLEAN" == "true" ]]; then
  echo "Cleaning previous build artifacts"
  rm -f CMakeCache.txt *.cmake
  rm -rf out
  rm -rf .buildtools
  # make clean
fi

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

# No fail on error
set +e

# Remove old package
rm -f *.deb *.rpm

# Build new package
sudo make package

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
