@echo off

cd %~dp0
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

echo Update all public submodules...
git -c submodule."lib/modules".update=none submodule update --init --recursive

if DEFINED GIT_PULL_TOKEN (
  rd /s /q lib\modules
  git clone https://%GIT_PULL_TOKEN%:x-oauth-basic@github.com/microsoft/cpp_client_telemetry_modules.git lib\modules
)

set GTEST_PATH=third_party\googletest
if NOT EXIST %GTEST_PATH%\CMakeLists.txt (
 git clone --depth 1 --branch release-1.11.0 https://github.com/google/googletest %GTEST_PATH%
)

set CUSTOM_PROPS=
if ("%~1"=="") goto skip
set CUSTOM_PROPS="/p:ForceImportBeforeCppTargets=%1"
echo Using custom properties file for the build:
echo %CUSTOM_PROPS%
:skip

if NOT DEFINED SKIP_MD_BUILD (
  REM DLL and static /MD build
  REM Release
  call tools\RunMsBuild.bat Win32 Release "sqlite:Rebuild,zlib:Rebuild,sqlite-uwp:Rebuild,win32-dll:Rebuild,win32-lib:Rebuild,net40:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild,win10-lib:Rebuild,Tests\gmock:Rebuild,Tests\gtest:Rebuild,Tests\UnitTests:Rebuild,Tests\FuncTests:Rebuild" %CUSTOM_PROPS%
  call tools\RunMsBuild.bat x64 Release "sqlite:Rebuild,zlib:Rebuild,sqlite-uwp:Rebuild,win32-dll:Rebuild,win32-lib:Rebuild,net40:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild,win10-lib:Rebuild,Tests\gmock:Rebuild,Tests\gtest:Rebuild,Tests\UnitTests:Rebuild,Tests\FuncTests:Rebuild" %CUSTOM_PROPS%
  REM Debug
  if NOT DEFINED SKIP_DEBUG_BUILD (
    call tools\RunMsBuild.bat Win32 Debug "sqlite:Rebuild,zlib:Rebuild,sqlite-uwp:Rebuild,win32-dll:Rebuild,win32-lib:Rebuild,net40:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild,win10-lib:Rebuild,Tests\gmock:Rebuild,Tests\gtest:Rebuild,Tests\UnitTests:Rebuild,Tests\FuncTests:Rebuild" %CUSTOM_PROPS%
    call tools\RunMsBuild.bat x64 Debug "sqlite:Rebuild,zlib:Rebuild,sqlite-uwp:Rebuild,win32-dll:Rebuild,win32-lib:Rebuild,net40:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild,win10-lib:Rebuild,Tests\gmock:Rebuild,Tests\gtest:Rebuild,Tests\UnitTests:Rebuild,Tests\FuncTests:Rebuild" %CUSTOM_PROPS%
  )
)

if NOT DEFINED SKIP_MT_BUILD (
  REM Static /MT build
  REM Release
  call tools\RunMsBuild.bat Win32 Release.vc14x.MT-sqlite "sqlite:Rebuild,zlib:Rebuild,win32-lib:Rebuild" %CUSTOM_PROPS%
  call tools\RunMsBuild.bat x64 Release.vc14x.MT-sqlite "sqlite:Rebuild,zlib:Rebuild,win32-lib:Rebuild" %CUSTOM_PROPS%
  REM Debug
  if NOT DEFINED SKIP_DEBUG_BUILD (
    call tools\RunMsBuild.bat Win32 Debug.vc14x.MT-sqlite "sqlite:Rebuild,zlib:Rebuild,win32-lib:Rebuild" %CUSTOM_PROPS%
    call tools\RunMsBuild.bat x64 Debug.vc14x.MT-sqlite "sqlite:Rebuild,zlib:Rebuild,win32-lib:Rebuild" %CUSTOM_PROPS%
  )
)

if NOT DEFINED SKIP_ARM_BUILD (
  REM ARM DLL build
  REM Release
  call tools\RunMsBuild.bat ARM Release "zlib:Rebuild,sqlite-uwp:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild" %CUSTOM_PROPS%
  if NOT DEFINED SKIP_DEBUG_BUILD (
    REM Debug
    call tools\RunMsBuild.bat ARM Debug "zlib:Rebuild,sqlite-uwp:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild" %CUSTOM_PROPS%
  )
)

if NOT DEFINED SKIP_ARM64_BUILD (
  REM ARM64 DLL build
  REM Release
  call tools\RunMsBuild.bat ARM64 Release "sqlite:Rebuild,zlib:Rebuild,sqlite-uwp:Rebuild,win32-dll:Rebuild,win32-lib:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild,win10-lib:Rebuild" %CUSTOM_PROPS%
  if NOT DEFINED SKIP_DEBUG_BUILD (
    REM Debug
    call tools\RunMsBuild.bat ARM64 Debug "sqlite:Rebuild,zlib:Rebuild,sqlite-uwp:Rebuild,win32-dll:Rebuild,win32-lib:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild,win10-lib:Rebuild" %CUSTOM_PROPS%
  )
)
