@echo off
echo Building 1DS SDK as Android OS module
pushd "%~dp0"

if "%ANDROID_BUILD_TOP%" == "" (
  echo ==                                                                                     ==
  echo == Please set ANDROID_BUILD_TOP to point to your Android OS build top directory.       ==
  echo ==                                                                                     ==
  echo == For example:                                                                        ==
  echo ==                                                                                     ==
  echo ==  set ANDROID_BUILD_TOP=A:/aosp                                                      ==
  echo ==                                                                                     ==
  echo == where `A:/aosp` - is a top-level directory you are executing `lunch` command from.  ==
  popd ==                                                                                     ==
  popd ==  set PARALLEL_BUILD=1		- to enable parallel build of static and shared libs  ==
  exit
)

if "%BUILD_TYPE%"              == "" set "BUILD_TYPE=Release"
if "%ANDROID_ABI%"             == "" set "ANDROID_ABI=x86_64"
if "%ANDROID_SDK_VERSION%"     == "" set "ANDROID_SDK_VERSION=23"
if "%ANDROID_SDK_ROOT%"        == "" set "ANDROID_SDK_ROOT=C:\Android\android-sdk"
if "%ANDROID_NDK_VERSION%"     == "" set "ANDROID_NDK_VERSION=21.3.6528147"
if "%ANDROID_CMAKE_VERSION%"   == "" set "ANDROID_CMAKE_VERSION=3.10.2.4988404"
if "%ANDROID_HOME%"            == "" set "ANDROID_HOME=%ANDROID_SDK_ROOT%"
if "%ANDROID_NDK%"             == "" set "ANDROID_NDK=%ANDROID_SDK_ROOT%\ndk\%ANDROID_NDK_VERSION%"
if "%ANDROID_NDK_HOME%"        == "" set "ANDROID_NDK_HOME=%ANDROID_NDK%"
if "%ANDROID_PRODUCT_NAME%"    == "" set "ANDROID_PRODUCT_NAME=generic_x86_64"
if "%ANDROID_TOOLCHAIN%"       == "" set "ANDROID_TOOLCHAIN=x86_64-linux-android"

echo ANDROID_ABI             = %ANDROID_ABI%
echo ANDROID_SDK_VERSION     = %ANDROID_SDK_VERSION%
echo ANDROID_SDK_ROOT        = %ANDROID_SDK_ROOT%
echo ANDROID_NDK_VERSION     = %ANDROID_NDK_VERSION%
echo ANDROID_CMAKE_VERSION   = %ANDROID_CMAKE_VERSION%
echo ANDROID_HOME            = %ANDROID_HOME%
echo ANDROID_NDK             = %ANDROID_NDK%
echo ANDROID_NDK_HOME        = %ANDROID_NDK_HOME%
echo ANDROID_PRODUCT_NAME    = %ANDROID_PRODUCT_NAME%
echo ANDROID_TOOLCHAIN       = %ANDROID_TOOLCHAIN%

REM Install Android tools if necessary
call tools\setup-buildtools-android.cmd

set "PATH=%ANDROID_SDK_ROOT%\cmake\%ANDROID_CMAKE_VERSION%\bin;%ANDROID_NDK%;%PATH%"
REM Prefer latest OS CMake instead of Android-bundled CMake, but point it to android.toolchain.cmake 
set "PATH=C:\Program Files\CMake\bin;%PATH%"

REM TODO: delete 'out' directory to recreate cmake files in case if configuration changed
mkdir out 2>NUL
mkdir out\static 2>NUL
mkdir out\shared 2>NUL

echo Building shared library...
pushd out\shared
cmake -GNinja ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_SYSTEM_PROCESSOR=%ANDROID_ABI% ^
      -DCMAKE_SYSTEM_NAME=Android ^
      -DANDROID_ABI=%ANDROID_ABI% ^
      -DCMAKE_ANDROID_ARCH_ABI=%ANDROID_ABI% ^
      -DCMAKE_ANDROID_NDK=%ANDROID_NDK% ^
      -DCMAKE_ANDROID_STL_TYPE=c++_static ^
      -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%/build/cmake/android.toolchain.cmake ^
      -DANDROID_TOOLCHAIN_NAME=%ANDROID_TOOLCHAIN% ^
      -DCMAKE_MAKE_PROGRAM=%ANDROID_SDK_ROOT%/cmake/%ANDROID_CMAKE_VERSION%/bin/ninja.exe ^
      -DANDROID_NATIVE_API_LEVEL=android-%MINSDKVERSION% ^
      -DANDROID_BUILD_TOP=%ANDROID_BUILD_TOP% ^
      -DANDROID_PRODUCT_NAME=%ANDROID_PRODUCT_NAME% ^
      -DBUILD_SHARED_LIBS=ON ^
      %* ^
      ..\..
if DEFINED PARALLEL_BUILD (
  start cmd.exe /c ninja
) else (
  ninja
)
popd

echo Building static library...
pushd out\static
cmake -GNinja ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_SYSTEM_PROCESSOR=%ANDROID_ABI% ^
      -DCMAKE_SYSTEM_NAME=Android ^
      -DANDROID_ABI=%ANDROID_ABI% ^
      -DCMAKE_ANDROID_ARCH_ABI=%ANDROID_ABI% ^
      -DCMAKE_ANDROID_NDK=%ANDROID_NDK% ^
      -DCMAKE_ANDROID_STL_TYPE=c++_static ^
      -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%/build/cmake/android.toolchain.cmake ^
      -DANDROID_TOOLCHAIN_NAME=%ANDROID_TOOLCHAIN% ^
      -DCMAKE_MAKE_PROGRAM=%ANDROID_SDK_ROOT%/cmake/%ANDROID_CMAKE_VERSION%/bin/ninja.exe ^
      -DANDROID_NATIVE_API_LEVEL=android-%MINSDKVERSION% ^
      -DANDROID_BUILD_TOP=%ANDROID_BUILD_TOP% ^
      -DANDROID_PRODUCT_NAME=%ANDROID_PRODUCT_NAME% ^
      -DBUILD_SHARED_LIBS=OFF ^
      %* ^
      ..\..
if DEFINED PARALLEL_BUILD (
  start cmd.exe /c ninja
) else (
  ninja
)
popd
