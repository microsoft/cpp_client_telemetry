@echo off
pushd "%~dp0"

REM Users may override the default %ANDROID_SDK_ROOT% location if necessary

if "%ANDROID_SDK_ROOT%"      == "" set "ANDROID_SDK_ROOT=C:\Android\android-sdk"
if "%ANDROID_NDK_VERSION%"   == "" set "ANDROID_NDK_VERSION=21.1.6352462"
if "%ANDROID_CMAKE_VERSION%" == "" set "ANDROID_CMAKE_VERSION=3.10.2.4988404"
if "%ANDROID_HOME%"          == "" set "ANDROID_HOME=%ANDROID_SDK_ROOT%"
if "%ANDROID_NDK%"           == "" set "ANDROID_NDK=%ANDROID_SDK_ROOT%\ndk\%ANDROID_NDK_VERSION%"
if "%ANDROID_NDK_HOME%"      == "" set "ANDROID_NDK_HOME=%ANDROID_NDK%"
REM Consider using %ANDROID_NDK_ROOT% environment variable

REM Install Android tools if necessary
call tools\setup-buildtools-android.cmd

set "PATH=%ANDROID_SDK_ROOT%\cmake\%ANDROID_CMAKE_VERSION%\bin;%ANDROID_NDK%;%PATH%"

echo Building SDK
pushd .\lib\android_build
call .\gradlew.bat assemble %*
popd

echo Building Tests
pushd .\lib\android_build
call .\gradlew.bat maesdk:test %*
popd

popd
