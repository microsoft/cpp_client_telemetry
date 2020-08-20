#!/bin/bash

export PATH=/usr/local/bin:$PATH

if [[ ! -z "${GIT_PULL_TOKEN}" ]]; then
  rm -rf lib/modules
  echo Git local settings:
  git config -l
  echo Git system settings:
  git config --system --list
  git config credential.helper store
  git clone https://${GIT_PULL_TOKEN}:x-oauth-basic@github.com/microsoft/cpp_client_telemetry_modules.git lib/modules
fi

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

if [ "$1" == "noroot" ] || [ "$2" == "noroot" ]; then
export NOROOT=true
fi

if [ "$1" == "release" ] || [ "$2" == "release" ]; then
BUILD_TYPE="Release"
else
BUILD_TYPE="Debug"
fi

if [ "$1" == "arm64" ] || [ "$2" == "arm64" ]; then
MAC_ARCH="arm64"
elif [ "$1" == "universal" ] || [ "$2" == "universal" ]; then
MAC_ARCH="universal"
else
MAC_ARCH="x86_64"
fi

# Set target MacOS minver
export MACOSX_DEPLOYMENT_TARGET=10.10

# Install build tools and recent sqlite3
FILE=.buildtools
OS_NAME=`uname -a`
if [ ! -f $FILE ]; then
case "$OS_NAME" in
 *Darwin*) tools/setup-buildtools-apple.sh ;;
 *Linux*)  [[ -z "$NOROOT" ]] && sudo tools/setup-buildtools.sh || echo "No root: skipping build tools installation." ;;
 *)        echo "WARNING: unsupported OS $OS_NAME , skipping build tools installation.."
esac
# Assume that the build tools have been successfully installed
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

# .deb package
if [ -f /usr/bin/dpkg ]; then
export CMAKE_PACKAGE_TYPE=deb
fi

# .rpm package
if [ -f /usr/bin/rpmbuild ]; then
export CMAKE_PACKAGE_TYPE=rpm
fi


# Fail on error
set -e


# TODO: pass custom build flags?
cmake -DMAC_ARCH=$MAC_ARCH -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE ..
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
make package

# Debian / Ubuntu / Raspbian
if [ -f /usr/bin/dpkg ]; then
# Install new package
[[ -z "$NOROOT" ]] && sudo dpkg -i *.deb || echo "No root: skipping package deployment."
fi

# RedHat / CentOS
if [ -f /usr/bin/rpmbuild ]; then
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
