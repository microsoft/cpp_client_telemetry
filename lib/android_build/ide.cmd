@echo off
pushd "%~dp0"
for /F "usebackq skip=2 tokens=1-2*" %%A in (`reg.exe QUERY "HKLM\Software\Android Studio" /v "Path" 2^>nul`) do (
  set AndroidStudioPath=%%C
)

echo Android Studio Path: %AndroidStudioPath%
set "PATH=%AndroidStudioPath%\bin;%PATH%"

if "%ANDROID_SDK_ROOT%"   == "" set "ANDROID_SDK_ROOT=C:\Android\android-sdk"
if "%ANDROID_HOME%"       == "" set "ANDROID_HOME=%ANDROID_SDK_ROOT%"
if "%ANDROID_NDK_VERSION%"== "" set "ANDROID_NDK_VERSION=21.4.7075529"
if "%ANDROID_NDK%"        == "" set "ANDROID_NDK=%ANDROID_SDK_ROOT%\ndk\%ANDROID_NDK_VERSION%"
if "%ANDROID_NDK_HOME%"   == "" set "ANDROID_NDK_HOME=%ANDROID_NDK%"

where studio.bat >NUL 2>NUL
if %ERRORLEVEL% neq 0 (
  echo Please add Android Studio "studio.bat" to PATH
) else (
  call studio.bat %~dp0
)
popd
