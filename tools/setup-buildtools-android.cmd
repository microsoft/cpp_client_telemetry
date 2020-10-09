if exist ".buildtools" (
  echo Skiping dependencies installation
) else (
  pushd "%~dp0"
  where choco >NUL 2>NUL
  if %ERRORLEVEL% neq 0 call "%~dp0\setup-choco.cmd"

  echo Installing dependencies [requires Admin / elevated Command prompt]
  echo ANDROID_SDK_ROOT = %ANDROID_SDK_ROOT%
  echo ANDROID_HOME     = %ANDROID_HOME%
  echo ANDROID_NDK      = %ANDROID_NDK%
  echo ANDROID_NDK_HOME = %ANDROID_NDK_HOME%

  if not exist "%USERPROFILE%\.android\repositories.cfg" (
    echo Creating default repositories.cfg ...
    copy NUL "%USERPROFILE%\.android\repositories.cfg"
  )

  REM Use chocolatey for basic deps
  choco install --no-progress -y android-sdk
  choco install --no-progress -y ninja

  REM Use sdkmanager for additional deps
  pushd %ANDROID_SDK_ROOT%\tools\bin
  echo y | call sdkmanager.bat --include_obsolete --verbose --sdk_root=%ANDROID_SDK_ROOT% "platforms;android-28" "sources;android-28"
  echo y | call sdkmanager.bat --include_obsolete --verbose --sdk_root=%ANDROID_SDK_ROOT% "platforms;android-29" "sources;android-29"
  echo y | call sdkmanager.bat --include_obsolete --verbose --sdk_root=%ANDROID_SDK_ROOT% "build-tools;29.0.3"
  echo y | call sdkmanager.bat --include_obsolete --verbose --sdk_root=%ANDROID_SDK_ROOT% "platform-tools"
  echo y | call sdkmanager.bat --install --include_obsolete --verbose --sdk_root=%ANDROID_SDK_ROOT% "ndk-bundle" "cmake;%ANDROID_CMAKE_VERSION%" "ndk;%ANDROID_NDK_VERSION%"
  popd
  popd
  copy NUL .buildtools
)
