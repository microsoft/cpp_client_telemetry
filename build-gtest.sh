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
cd googletest
set -evx
env | sort
rm -rf build
mkdir -p build || true
cd build
cmake -Dgtest_build_samples=ON \
      -Dgmock_build_samples=ON \
      -Dgtest_build_tests=$BUILD_TESTS \
      -Dgmock_build_tests=$BUILD_TESTS \
      -DCMAKE_CXX_FLAGS="-fPIC $CXX_FLAGS" \
      -DBUILD_IOS=$IOS_BUILD \
      ..
make
CTEST_OUTPUT_ON_FAILURE=1 make test
#make install
