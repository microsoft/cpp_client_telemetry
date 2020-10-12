# Build 1DS C++ SDK for Android OS platform modules

This tutorial guides you through the process of integrating the 1DS SDK into Android OS Platform as native module.

It assumes that your system is configued with the necessary Android essential build tools, compilers, CMake, clang, SDK, NDK.

Best experience is achieved using Linux or WSL. Windows machine may be used as well with remotely mounted Android OS build root.

## 1. Clone the repository

Run `git clone https://github.com/microsoft/cpp_client_telemetry.git --recurse-submodules` to clone the repo.

## 2. Build SDK

Please review the `build-android.cmd` build script that sets up the necessary paths to AOSP (Platform OS or Product) build tree:

```console
build-android.cmd os
```

Let's dissect an equivalent example of Linux or Mac shell script:

```console

# Setup paths to Android SDK, Android NDK, Android OS Build top

export ANDROID_SDK_ROOT=/android/sdk
export ANDROID_NDK_VERSION=21.3.6528147
export ANDROID_CMAKE_VERSION=3.10.2.4988404
export ANDROID_HOME=$ANDROID_SDK_ROOT
export ANDROID_NDK=${ANDROID_SDK_ROOT}/ndk/${ANDROID_NDK_VERSION}
export ANDROID_NDK_HOME=${ANDROID_NDK}

# Please specify your product target platform. This example is for x86_64 Emulator:
export ABI=x86_64
export MINSDKVERSION=23

# This is the root folder of the platform build tree
export ANDROID_BUILD_TOP=/android/aosp/

mkdir out
cd out

# We ninja generator - for Ninja bundled with NDK, but make should work too.
# Note the product name - `generic_x86_64` (Emulator).

cmake -GNinja	\
	-DCMAKE_SYSTEM_PROCESSOR=$ABI	\
	-DCMAKE_SYSTEM_NAME=Android	\
	-DANDROID_ABI=$ABI	\
	-DCMAKE_ANDROID_ARCH_ABI=$ABI	\
	-DCMAKE_ANDROID_NDK=$ANDROID_NDK	\
	-DCMAKE_ANDROID_STL_TYPE=c++_static	\
	-DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake	\
	-DANDROID_TOOLCHAIN_NAME=x86_64-linux-android	\
	-DCMAKE_MAKE_PROGRAM=${ANDROID_SDK_ROOT}/cmake/${ANDROID_CMAKE_VERSION}/bin/ninja	\
	-DCMAKE_SYSTEM_PROCESSOR=$ABI	\
	-DANDROID_NATIVE_API_LEVEL=android-${MINSDKVERSION}	\
	-DANDROID_BUILD_TOP=${ANDROID_BUILD_TOP}	\
	-DANDROID_PRODUCT_NAME=generic_x86_64	\
	..

# Make sure ninja from Android NDK bundle is in the path

ninja
```

This will build a static SDK library and place the output in `out/lib/libmat.a` 

## 3. Integrate the SDK into your C++ project

Please refer to [examples/cpp/EventSender](../examples/cpp/EventSender) example that hows how to build native binary targeting Android OS (Platform layer) using NDK and cmake.
