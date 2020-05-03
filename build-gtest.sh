#!/usr/bin/env bash
cd `dirname $0`

# Use latest Google Test
if [ -f /etc/os-release ]; then
  source /etc/os-release
  # Use new Google Test on latest Ubuntu 20.04 : old one no longer compiles on 20
  if [ "$VERSION_ID" == "20.04" ]; then
    echo Running on Ubuntu 20.04
    echo Clone googletest from google/googletest:master ...
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
cmake -Dgtest_build_samples=OFF \
      -Dgmock_build_samples=OFF \
      -Dgtest_build_tests=OFF \
      -Dgmock_build_tests=OFF \
      -DCMAKE_CXX_FLAGS="-fPIC $CXX_FLAGS" \
      ..
make
# CTEST_OUTPUT_ON_FAILURE=1 make test
# make install
