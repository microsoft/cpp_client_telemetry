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
  "%VSINSTALLER%" modify --installPath "%VSINSTALLDIR%" -q ^
--add Microsoft.Component.MSBuild ^
--add Microsoft.VisualStudio.Component.Roslyn.Compiler ^
--add Microsoft.VisualStudio.Component.TextTemplating ^
--add Microsoft.VisualStudio.Component.VC.14.20.ARM ^
--add Microsoft.VisualStudio.Component.VC.14.20.ARM64 ^
--add Microsoft.VisualStudio.Component.VC.14.20.ATL ^
--add Microsoft.VisualStudio.Component.VC.14.20.ATL.ARM ^
--add Microsoft.VisualStudio.Component.VC.14.20.ATL.ARM64 ^
--add Microsoft.VisualStudio.Component.VC.14.20.CLI.Support ^
--add Microsoft.VisualStudio.Component.VC.ATL ^
--add Microsoft.VisualStudio.Component.VC.ATL.ARM ^
--add Microsoft.VisualStudio.Component.VC.ATL.ARM64 ^
--add Microsoft.VisualStudio.Component.VC.ATLMFC ^
--add Microsoft.VisualStudio.Component.VC.CLI.Support ^
--add Microsoft.VisualStudio.Component.VC.CoreBuildTools ^
--add Microsoft.VisualStudio.Component.VC.CoreIde ^
--add Microsoft.VisualStudio.Component.VC.Llvm.Clang ^
--add Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset ^
--add Microsoft.VisualStudio.Component.VC.Modules.x86.x64 ^
--add Microsoft.VisualStudio.Component.VC.Redist.14.Latest ^
--add Microsoft.VisualStudio.Component.VC.Tools.14.11 ^
--add Microsoft.VisualStudio.Component.VC.Tools.ARM ^
--add Microsoft.VisualStudio.Component.VC.Tools.ARM64 ^
--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
--add Microsoft.VisualStudio.Component.VC.v141.ARM ^
--add Microsoft.VisualStudio.Component.VC.v141.ARM64 ^
--add Microsoft.VisualStudio.Component.VC.v141.x86.x64 ^
--add Microsoft.VisualStudio.Component.Windows10SDK ^
--add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core ^
--add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Llvm.Clang ^
--add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Win81 ^
--add Microsoft.VisualStudio.ComponentGroup.UWP.VC.BuildTools ^
--add Microsoft.VisualStudio.ComponentGroup.UWP.VC.v141.BuildTools ^
--add Microsoft.VisualStudio.Workload.VCTools ^
--add Microsoft.VisualStudio.Component.UWP.VC.ARM64 ^
--norestart --noUpdateInstaller
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

REM Required for LLVM Clang build on Windows
call install-llvm.cmd

popd
exit /b 0
