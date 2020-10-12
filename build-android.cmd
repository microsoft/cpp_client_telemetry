@echo off
pushd "%~dp0"

REM Users may override the default %ANDROID_SDK_ROOT% location if necessary

if "%ANDROID_SDK_ROOT%"      == "" set "ANDROID_SDK_ROOT=C:\Android\android-sdk"
if "%ANDROID_NDK_VERSION%"   == "" set "ANDROID_NDK_VERSION=21.3.6528147"
if "%ANDROID_CMAKE_VERSION%" == "" set "ANDROID_CMAKE_VERSION=3.10.2.4988404"
if "%ANDROID_HOME%"          == "" set "ANDROID_HOME=%ANDROID_SDK_ROOT%"
if "%ANDROID_NDK%"           == "" set "ANDROID_NDK=%ANDROID_SDK_ROOT%\ndk\%ANDROID_NDK_VERSION%"
if "%ANDROID_NDK_HOME%"      == "" set "ANDROID_NDK_HOME=%ANDROID_NDK%"
REM Consider using %ANDROID_NDK_ROOT% environment variable

REM Install Android tools if necessary
call tools\setup-buildtools-android.cmd

set "PATH=%ANDROID_SDK_ROOT%\cmake\%ANDROID_CMAKE_VERSION%\bin;%ANDROID_NDK%;%PATH%"

if "%1" == "os" (
  echo Building SDK as OS module
  REM set "PATH=%ANDROID_SDK_ROOT%\cmake\%ANDROID_CMAKE_VERSION%\bin;%PATH%"
  REM Don't use Android CMake
  set "PATH=C:\Program Files\CMake\bin;%PATH%"

  REM TODO: delete 'out' directory to recreate cmake files in case if configuration changed
  mkdir out 2>NUL
  cd out

  REM TODO: override ABI to whatever ABI user wants to build it for
  set ABI=x86_64
  set MINSDKVERSION=23
  set ANDROID_BUILD_TOP=%2 

REM TODO: This disaster works only for arm, need to figure out how to override it.
REM -DANDROID_PLATFORM=android-%MINSDKVERSION% ^

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
        -DANDROID_BUILD_TOP=A:/aosp ^
        -DANDROID_PRODUCT_NAME=generic_x86_64 ^
        -DANDROID_OS_EXTRA_LIBS=A:/openssl-curl-android/build ^
	..
  ninja

) else (
  echo Building SDK as App module
  pushd .\lib\android_build
  call .\gradlew.bat assemble %*
  popd

  echo Building Tests
  pushd .\lib\android_build
  call .\gradlew.bat maesdk:test %*
  popd

  popd
)
