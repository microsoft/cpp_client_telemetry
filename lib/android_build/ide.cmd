@echo off
pushd "%~dp0"
for /F "usebackq skip=2 tokens=1-2*" %%A in (`reg.exe QUERY "HKLM\Software\Android Studio" /v "Path" 2^>nul`) do (
  set AndroidStudioPath=%%C
)

echo Android Studio Path: %AndroidStudioPath%
set "PATH=%AndroidStudioPath%\bin;%PATH%"
if "%ANDROID_SDK_ROOT%"== "" (
  echo Set default Android SDK path ...
  set "ANDROID_SDK_ROOT=C:\Android\android-sdk"
  set "ANDROID_HOME=%ANDROID_SDK_ROOT%"
  set "ANDROID_NDK=%ANDROID_SDK_ROOT%"\ndk\20.0.5594570"
  set "ANDROID_NDK_HOME=%ANDROID_NDK%"
)

where studio.bat >NUL 2>NUL
if %ERRORLEVEL% neq 0 (
  echo Please add Android Studio "studio.bat" to PATH
) else (
  call studio.bat %~dp0
)
popd
