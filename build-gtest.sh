#!/usr/bin/env bash
cd `dirname $0`

# Use latest Google Test
if [ -f /etc/os-release ]; then
  source /etc/os-release
  # Use new Google Test on latest Ubuntu 20.04 : old one no longer compiles on 20
  if [ "$VERSION_ID" == "20.04" ]; then
    rm -rf googletest
    git clone https://github.com/google/googletest
  fi
fi

cd googletest
set -evx
env | sort
rm -rf build
mkdir -p build || true
cd build
cmake -Dgtest_build_samples=ON \
      -Dgmock_build_samples=ON \
      -Dgtest_build_tests=ON \
      -Dgmock_build_tests=ON \
      -DCMAKE_CXX_FLAGS="-fPIC $CXX_FLAGS" \
      ..
make
# CTEST_OUTPUT_ON_FAILURE=1 make test
# make install
