@echo off
set "BUILD_TOOLS_MARKER=.buildtools"
set "BUILD_TOOLS_MARKER_VERSION=android-cmake-%ANDROID_CMAKE_VERSION%-ndk-%ANDROID_NDK_VERSION%"
set "BUILD_TOOLS_MARKER_ACTUAL="

if not exist "%BUILD_TOOLS_MARKER%" goto install
set /p BUILD_TOOLS_MARKER_ACTUAL=<"%BUILD_TOOLS_MARKER%"
if /I not "%BUILD_TOOLS_MARKER_ACTUAL%"=="%BUILD_TOOLS_MARKER_VERSION%" goto install
if not exist "%ANDROID_SDK_ROOT%\cmake\%ANDROID_CMAKE_VERSION%\bin\cmake.exe" goto install
if not exist "%ANDROID_NDK%" goto install
echo Skipping dependencies installation
exit /b 0

:install
del "%BUILD_TOOLS_MARKER%" 2>NUL
pushd "%~dp0"
where choco >NUL 2>NUL
if errorlevel 1 call "%~dp0\setup-choco.cmd"
if errorlevel 1 goto install_failed

echo Installing dependencies [requires Admin / elevated Command prompt]
echo ANDROID_SDK_ROOT = %ANDROID_SDK_ROOT%
echo ANDROID_HOME     = %ANDROID_HOME%
echo ANDROID_NDK      = %ANDROID_NDK%
echo ANDROID_NDK_HOME = %ANDROID_NDK_HOME%

if not exist "%USERPROFILE%\.android\repositories.cfg" (
  echo Creating default repositories.cfg ...
  copy NUL "%USERPROFILE%\.android\repositories.cfg"
  if errorlevel 1 goto install_failed
)

REM Use chocolatey for basic deps
call choco install --no-progress -y android-sdk
if errorlevel 1 goto install_failed
call choco install --no-progress -y ninja
if errorlevel 1 goto install_failed

REM Use sdkmanager for additional deps
pushd "%ANDROID_SDK_ROOT%\tools\bin"
echo y | call .\sdkmanager.bat --include_obsolete --verbose --sdk_root="%ANDROID_SDK_ROOT%" "platforms;android-28" "sources;android-28"
if errorlevel 1 goto sdkmanager_failed
echo y | call .\sdkmanager.bat --include_obsolete --verbose --sdk_root="%ANDROID_SDK_ROOT%" "platforms;android-29" "sources;android-29"
if errorlevel 1 goto sdkmanager_failed
echo y | call .\sdkmanager.bat --include_obsolete --verbose --sdk_root="%ANDROID_SDK_ROOT%" "build-tools;29.0.3"
if errorlevel 1 goto sdkmanager_failed
echo y | call .\sdkmanager.bat --include_obsolete --verbose --sdk_root="%ANDROID_SDK_ROOT%" "platform-tools"
if errorlevel 1 goto sdkmanager_failed
echo y | call .\sdkmanager.bat --install --include_obsolete --verbose --sdk_root="%ANDROID_SDK_ROOT%" "ndk-bundle" "cmake;%ANDROID_CMAKE_VERSION%" "ndk;%ANDROID_NDK_VERSION%"
if errorlevel 1 goto sdkmanager_failed
popd
popd

if not exist "%ANDROID_SDK_ROOT%\cmake\%ANDROID_CMAKE_VERSION%\bin\cmake.exe" exit /b 1
if not exist "%ANDROID_NDK%" exit /b 1
>"%BUILD_TOOLS_MARKER%" echo %BUILD_TOOLS_MARKER_VERSION%
exit /b 0

:sdkmanager_failed
popd
:install_failed
popd
exit /b 1
