REM @echo off
set "PATH=%PATH%;%~dp0;%~dp0\vcpkg"
pushd %~dp0

REM Fail if chocolatey is not installed
where /Q choco
if ERRORLEVEL 1 (
  echo This script requires chocolatey. Installation instructions: https://chocolatey.org/docs/installation
  exit -1
)

REM Print current Visual Studio installations detected
where /Q vswhere
if ERRORLEVEL 0 (
  echo Visual Studio installations detected:
  vswhere -property installationPath
)

REM Install tools needed to build SDK with either Visual Studio or CMake
choco install -y cmake svn git llvm zip

REM Try to autodetect Visual Studio
call "%~dp0\vcvars.cmd"
if "%VSTOOLS_NOTFOUND%" == "1" (
  REM Cannot detect MSBuild path
  REM TODO: no command line tools..
  REM TODO: use MSBuild from vswhere?
)
echo Visual Studio installation directory:
echo %VSINSTALLDIR%

set "VSINSTALLER=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vs_installer.exe"
if NOT exist "%VSINSTALLER%" (
  call download.cmd https://aka.ms/vs/16/release/vs_buildtools.exe
  set VSINSTALLER=vs_buildtools.exe
)
echo Visual Studio installer:
echo %VSINSTALLER%

REM Install optional components required for ARM build - vs2017-BuildTools
if exist "%VSINSTALLDIR%" (
  echo Running Visual Studio installer..
  "%VSINSTALLER%" modify --installPath "%VSINSTALLDIR%" --config "%~dp0\.vsconfig.%VSVERSION%" --force --quiet --norestart
)

where /Q vcpkg.exe
if ERRORLEVEL 1 (
  REM Build our own vcpkg from source
  pushd .\vcpkg
  call bootstrap-vcpkg.bat
  popd
)

REM Install it
vcpkg integrate install
vcpkg install gtest:x64-windows
vcpkg install --overlay-ports=%~dp0\ports benchmark:x64-windows
vcpkg install ms-gsl:x64-windows

if DEFINED INSTALL_LLVM (
  REM Required for LLVM Clang build on Windows
  call install-llvm.cmd
)

popd
exit /b 0
