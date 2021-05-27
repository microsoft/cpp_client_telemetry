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

set CUSTOM_PROPS=
if ("%~1"=="") goto skip
set CUSTOM_PROPS="/p:ForceImportBeforeCppTargets=%1"
echo Using custom properties file for the build:
echo %CUSTOM_PROPS%
:skip

if NOT DEFINED SKIP_MD_BUILD (
  REM DLL and static /MD build
  REM Release
  call tools\RunMsBuild.bat Win32 Release "sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests" %CUSTOM_PROPS%
  call tools\RunMsBuild.bat x64 Release "sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests" %CUSTOM_PROPS%
  REM Debug
  if NOT DEFINED SKIP_DEBUG_BUILD (
    call tools\RunMsBuild.bat Win32 Debug "sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests" %CUSTOM_PROPS%
    call tools\RunMsBuild.bat x64 Debug "sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests" %CUSTOM_PROPS%
  )
)

if NOT DEFINED SKIP_MT_BUILD (
  REM Static /MT build
  REM Release
  call tools\RunMsBuild.bat Win32 Release.vc14x.MT-sqlite "sqlite,zlib,win32-lib" %CUSTOM_PROPS%
  call tools\RunMsBuild.bat x64 Release.vc14x.MT-sqlite "sqlite,zlib,win32-lib" %CUSTOM_PROPS%
  REM Debug
  if NOT DEFINED SKIP_DEBUG_BUILD (
    call tools\RunMsBuild.bat Win32 Debug.vc14x.MT-sqlite "sqlite,zlib,win32-lib" %CUSTOM_PROPS%
    call tools\RunMsBuild.bat x64 Debug.vc14x.MT-sqlite "sqlite,zlib,win32-lib" %CUSTOM_PROPS%
  )
)

if NOT DEFINED SKIP_ARM_BUILD (
  REM ARM DLL build
  REM Release
  call tools\RunMsBuild.bat ARM Release "zlib,sqlite-uwp,win10-cs,win10-dll" %CUSTOM_PROPS%
  if NOT DEFINED SKIP_DEBUG_BUILD (
    REM Debug
    call tools\RunMsBuild.bat ARM Debug "zlib,sqlite-uwp,win10-cs,win10-dll" %CUSTOM_PROPS%
  )
)

if NOT DEFINED SKIP_ARM64_BUILD (
  REM ARM64 DLL build
  REM Release
  call tools\RunMsBuild.bat ARM64 Release "sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,win10-cs,win10-dll" %CUSTOM_PROPS%
  if NOT DEFINED SKIP_DEBUG_BUILD (
    REM Debug
    call tools\RunMsBuild.bat ARM64 Debug "sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,win10-cs,win10-dll" %CUSTOM_PROPS%
  )
)
