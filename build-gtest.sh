#!/usr/bin/env bash
ARCH="x86_64"
if [ "$1" == "ios" ]; then
  IOS_BUILD="YES"
  # Skip building tests on iOS as there is no way to run them
  BUILD_TESTS="OFF"
else
  IOS_BUILD="NO"
  if [ "$1" == "arm64" ]; then
    ARCH="arm64"
    BUILD_TESTS="OFF"
  else
    BUILD_TESTS="ON"
  fi
fi

cd `dirname $0`

GTEST_PATH=googletest
USE_LATEST_GTEST="false"
OS_NAME=`uname -s`
#TODO: switch all OS builds to Google Test located in third_party/googletest submodule
case "$OS_NAME" in
Darwin) 
  mac_os_ver=$(sw_vers -productVersion)
  IFS='.' read -r -a mac_ver_first_octet <<< "$mac_os_ver"
  # Use new Google Test on macOS 11.0 or higher ; old one no longer compiles on 11.0
  if [[ "$mac_ver_first_octet" -ge 11 ]] ; then
    echo "running on Mac OS 11.0 or higher"
    USE_LATEST_GTEST="true"
  else
    echo "running older MacOS $mac_os_ver"
  fi
  ;;
Linux)
  source /etc/os-release
  echo $VERSION_ID
  # Use new Google Test on latest Ubuntu 20.04 : old one no longer compiles on 20
  if [ "$VERSION_ID" == "20.04" ]; then
    echo "Running on Ubuntu 20.04"
    USE_LATEST_GTEST="true"
  fi
  ;;
esac

if [ "$USE_LATEST_GTEST" == "true" ]; then
  echo "Using latest googletest"
  lsinclude=`ls -l third_party/googletest`
  GTEST_PATH=third_party/googletest
  if [ ! "$(ls -A $GTEST_PATH/CMakeLists.txt)" ]; then 
    echo Clone googletest from google/googletest:master ...
    rm -rf ${GTEST_PATH} #delete just if empty directory exists
    git clone https://github.com/google/googletest $GTEST_PATH
  else
    echo "Using existing googletest from thirdparty/"
  fi
else
  echo "Using existing(older) googletest from repo root"
fi
pushd $GTEST_PATH
set -evx
env | sort
rm -rf build
mkdir -p build || true
cd build
cmake -Dgtest_build_samples=OFF \
      -Dgmock_build_samples=OFF \
      -Dgtest_build_tests=OFF \
      -Dgmock_build_tests=OFF \
      -DCMAKE_CXX_FLAGS="-fPIC $CXX_FLAGS" \
      -DBUILD_IOS=$IOS_BUILD \
      -DARCH=$ARCH \
      ..
make
popd
# CTEST_OUTPUT_ON_FAILURE=1 make test
# make install
