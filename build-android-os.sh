#!/bin/bash
echo Building 1DS SDK as Android OS module
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

pushd $DIR
if [ "$ANDROID_BUILD_TOP" = "" ]; then
  echo "========================================================================================="
  echo "==                                                                                     =="
  echo "== Please set ANDROID_BUILD_TOP to point to your Android OS build top directory.       =="
  echo "==                                                                                     =="
  echo "== For example:                                                                        =="
  echo "==                                                                                     =="
  echo "==  export ANDROID_BUILD_TOP=/android/aosp                                             =="
  echo "==                                                                                     =="
  echo "== where '/android/aosp' is top-level dir you are executing 'lunch' command from.      =="
  echo "== See: https://source.android.com/setup/build/downloading                             =="
  echo "==                                                                                     =="
  echo "========================================================================================="
  popd
  exit
fi

export BUILD_TYPE=${BUILD_TYPE:-Release}
export ANDROID_ABI=${ANDROID_ABI:-x86_64}
export ANDROID_SDK_VERSION=${ANDROID_SDK_VERSION:-23}
export ANDROID_SDK_ROOT=${ANDROID_SDK_ROOT:-/usr/lib/android-sdk}
export ANDROID_NDK_VERSION=${ANDROID_NDK_VERSION:-21.3.6528147}
export ANDROID_CMAKE_VERSION=${ANDROID_CMAKE_VERSION:-3.10.2.4988404}
export ANDROID_HOME=${ANDROID_SDK_ROOT}
# In this case the NDK is extracted into /usr/lib/android-sdk!
export ANDROID_NDK=${ANDROID_SDK_ROOT}/android-ndk-r21d
export ANDROID_NDK_HOME=${ANDROID_NDK}
export ANDROID_PRODUCT_NAME=generic_x86_64
export ANDROID_TOOLCHAIN=x86_64-linux-android

echo "ANDROID_ABI             = $ANDROID_ABI"
echo "ANDROID_SDK_VERSION     = $ANDROID_SDK_VERSION"
echo "ANDROID_SDK_ROOT        = $ANDROID_SDK_ROOT"
echo "ANDROID_NDK_VERSION     = $ANDROID_NDK_VERSION"
echo "ANDROID_CMAKE_VERSION   = $ANDROID_CMAKE_VERSION"
echo "ANDROID_HOME            = $ANDROID_HOME"
echo "ANDROID_NDK             = $ANDROID_NDK"
echo "ANDROID_NDK_HOME        = $ANDROID_NDK_HOME"
echo "ANDROID_PRODUCT_NAME    = $ANDROID_PRODUCT_NAME"
echo "ANDROID_TOOLCHAIN       = $ANDROID_TOOLCHAIN"

mkdir -p out
mkdir -p out/static
mkdir -p out/shared

export PATH=/usr/local/bin:$PATH

echo Building shared library...
pushd out/shared
cmake -G Ninja \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_SYSTEM_PROCESSOR=$ANDROID_ABI \
      -DCMAKE_SYSTEM_NAME=Android \
      -DANDROID_ABI=$ANDROID_ABI \
      -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI \
      -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
      -DCMAKE_ANDROID_STL_TYPE=c++_static \
      -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
      -DANDROID_TOOLCHAIN_NAME=$ANDROID_TOOLCHAIN \
      -DCMAKE_MAKE_PROGRAM=`which ninja` \
      -DANDROID_NATIVE_API_LEVEL=android-${MINSDKVERSION} \
      -DANDROID_BUILD_TOP=${ANDROID_BUILD_TOP} \
      -DANDROID_PRODUCT_NAME=${ANDROID_PRODUCT_NAME} \
      -DBUILD_SHARED_LIBS=ON \
      "$@" \
      ../..
ninja
popd

echo Building static library...
pushd out/static
cmake -G Ninja \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_SYSTEM_PROCESSOR=$ANDROID_ABI \
      -DCMAKE_SYSTEM_NAME=Android \
      -DANDROID_ABI=$ANDROID_ABI \
      -DCMAKE_ANDROID_ARCH_ABI=$ANDROID_ABI \
      -DCMAKE_ANDROID_NDK=$ANDROID_NDK \
      -DCMAKE_ANDROID_STL_TYPE=c++_static \
      -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
      -DANDROID_TOOLCHAIN_NAME=${ANDROID_TOOLCHAIN} \
      -DCMAKE_MAKE_PROGRAM=`which ninja` \
      -DANDROID_NATIVE_API_LEVEL=android-${MINSDKVERSION} \
      -DANDROID_BUILD_TOP=${ANDROID_BUILD_TOP} \
      -DANDROID_PRODUCT_NAME=${ANDROID_PRODUCT_NAME} \
      -DBUILD_SHARED_LIBS=OFF \
      "$@" \
      ../..
ninja
popd

popd
