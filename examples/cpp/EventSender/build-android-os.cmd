REM
REM This build script illustrates how to build EventSender example as native OS module using NDK
REM Target device: Emulator x86_64
REM Android OS build top directory: `A:/aosp`
REM
@echo off
pushd "%~dp0"

set ABI=x86_64
set MINSDKVERSION=23
set ANDROID_BUILD_TOP=A:/aosp
set ANDROID_PRODUCT_NAME=generic_x86_64

if "%ANDROID_SDK_ROOT%"      == "" set "ANDROID_SDK_ROOT=C:\Android\android-sdk"
if "%ANDROID_NDK_VERSION%"   == "" set "ANDROID_NDK_VERSION=21.3.6528147"
if "%ANDROID_CMAKE_VERSION%" == "" set "ANDROID_CMAKE_VERSION=3.10.2.4988404"
if "%ANDROID_HOME%"          == "" set "ANDROID_HOME=%ANDROID_SDK_ROOT%"
if "%ANDROID_NDK%"           == "" set "ANDROID_NDK=%ANDROID_SDK_ROOT%\ndk\%ANDROID_NDK_VERSION%"
if "%ANDROID_NDK_HOME%"      == "" set "ANDROID_NDK_HOME=%ANDROID_NDK%"

set "PATH=%ANDROID_SDK_ROOT%\cmake\%ANDROID_CMAKE_VERSION%\bin;%ANDROID_NDK%;%PATH%"
REM Requires x86_64 installation of CMake!
set "PATH=C:\Program Files\CMake\bin;%PATH%"

if not exist out mkdir out
cd out

REM Requires ninja.exe from Android ndk-bundle
cmake -GNinja ^
	-DCMAKE_SYSTEM_PROCESSOR=%ABI% ^
	-DCMAKE_SYSTEM_NAME=Android ^
	-DANDROID_ABI=%ABI% ^
	-DCMAKE_ANDROID_ARCH_ABI=%ABI% ^
	-DCMAKE_ANDROID_NDK=%ANDROID_NDK% ^
	-DCMAKE_ANDROID_STL_TYPE=c++_static ^
	-DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%/build/cmake/android.toolchain.cmake ^
	-DANDROID_TOOLCHAIN_NAME=x86_64-linux-android ^
	-DCMAKE_MAKE_PROGRAM=%ANDROID_SDK_ROOT%/cmake/%ANDROID_CMAKE_VERSION%/bin/ninja.exe ^
	-DCMAKE_SYSTEM_PROCESSOR=%ABI% ^
	-DANDROID_NATIVE_API_LEVEL=android-%MINSDKVERSION% ^
	-DANDROID_BUILD_TOP=%ANDROID_BUILD_TOP% ^
	-DANDROID_PRODUCT_NAME=%ANDROID_PRODUCT_NAME% ^
	..
ninja

popd
