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

# Use latest Google Test for Ubuntu 20.04
# TODO: switch all OS builds to Google Test located in third_party/googletest submodule
if [ -f /etc/os-release ]; then
  source /etc/os-release
  # Use new Google Test on latest Ubuntu 20.04 : old one no longer compiles on 20
  if [ "$VERSION_ID" == "20.04" ]; then
    echo Running on Ubuntu 20.04
    GTEST_PATH=third_party/googletest
    if [ ! "$(ls -A $GTEST_PATH)" ]; then      
      echo Clone googletest from google/googletest:master ...
      git clone https://github.com/google/googletest $GTEST_PATH
    fi
  fi
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
