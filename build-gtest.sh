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
pwd
ls third_party/googletest
GTEST_PATH=third_party/googletest
if [ ! "$(ls -A $GTEST_PATH/CMakeLists.txt)" ]; then 
  echo Clone googletest from google/googletest:master ...
  rm -rf ${GTEST_PATH} #delete just if empty directory exists
  git clone --depth 1 --branch release-1.12.1 https://github.com/google/googletest $GTEST_PATH
else
  echo "Using existing googletest from thirdparty/"
fi

echo "Add ios and arm64 build steps for googletest"
cat > $GTEST_PATH/CMakeLists_temp.txt << EOF
# If building for iOS, set all the iOS options
if(BUILD_IOS)
  message("-- Building for iOS simulator..")
  set(TARGET_ARCH "APPLE") 
  set(IOS True)
  set(APPLE True)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "Force unset of the deployment target for iOS" FORCE)
  set(CMAKE_C_FLAGS "\${CMAKE_C_FLAGS} -miphoneos-version-min=10.0")
  set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -miphoneos-version-min=10.0 -std=c++11")
  set(IOS_PLATFORM "iphonesimulator")
  set(CMAKE_SYSTEM_PROCESSOR x86_64)
  execute_process(COMMAND xcodebuild -version -sdk \${IOS_PLATFORM} Path
    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT_OUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(CMAKE_OSX_SYSROOT \${CMAKE_OSX_SYSROOT_OUT} CACHE STRING "Force set of the sysroot for iOS" FORCE)
  message("-- CMAKE_OSX_SYSROOT       \${CMAKE_OSX_SYSROOT}")
elseif(\${ARCH} STREQUAL "arm64")
  set(CMAKE_C_FLAGS "\${CMAKE_C_FLAGS} -arch arm64")
  set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -arch arm64")
  set(CMAKE_SYSTEM_PROCESSOR arm64)
  set(APPLE True)
endif()
EOF

sed -i -e "/^cmake_minimum_required/r $GTEST_PATH/CMakeLists_temp.txt"  $GTEST_PATH/CMakeLists.txt
rm $GTEST_PATH/CMakeLists_temp.txt

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

lipo -info /Users/runner/work/cpp_client_telemetry/cpp_client_telemetry/third_party/googletest/build/lib/libgtest.a
popd
# CTEST_OUTPUT_ON_FAILURE=1 make test
# make install
