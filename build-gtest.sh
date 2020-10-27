#!/usr/bin/env bash
if [ "$1" == "ios" ]; then
  IOS_BUILD="YES"
  # Skip building tests on iOS as there is no way to run them
  BUILD_TESTS="OFF"
else
  IOS_BUILD="NO"
  BUILD_TESTS="ON"
fi

cd `dirname $0`

GTEST_PATH=third_party/googletest
if [ ! "$(ls -A $GTEST_PATH)" ]; then      
  echo Clone googletest from google/googletest:master ...
  git clone https://github.com/google/googletest $GTEST_PATH
fi

pushd $GTEST_PATH
set -evx
env | sort
rm -rf build
mkdir -p build || true
cd build
cmake -Dgtest_build_samples=OFF \
      -Dgmock_build_samples=OFF \
      -Dgtest_build_tests=$BUILD_TESTS \
      -Dgmock_build_tests=$BUILD_TESTS \
      -DCMAKE_CXX_FLAGS="-fPIC $CXX_FLAGS" \
      -DBUILD_IOS=$IOS_BUILD \
      ..
make
popd
# CTEST_OUTPUT_ON_FAILURE=1 make test
# make install
