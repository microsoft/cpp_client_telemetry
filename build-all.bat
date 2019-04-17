@echo off
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

echo "Building using Visual Studio 2017 tools"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=Solutions\MSTelemetrySDK.sln

set CUSTOM_PROPS=
if ("%1"=="") goto skip
set CUSTOM_PROPS="/p:ForceImportBeforeCppTargets=%1"
echo Using custom properties file for the build:
echo %CUSTOM_PROPS%
:skip

REM DLL and static /MD build
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=x64 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=x64 %CUSTOM_PROPS%

REM Static /MT build
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug.vs2015.MT-sqlite /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release.vs2015.MT-sqlite /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug.vs2015.MT-sqlite /p:Platform=x64 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release.vs2015.MT-sqlite /p:Platform=x64 %CUSTOM_PROPS%

REM ARM DLL build
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\vsdevcmd\ext\vcvars.bat" arm
msbuild %SOLUTION% /target:zlib,sqlite-uwp,win10-cs,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=ARM %CUSTOM_PROPS%
msbuild %SOLUTION% /target:zlib,sqlite-uwp,win10-cs,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=ARM %CUSTOM_PROPS%

REM ARM64 DLL build
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\vsdevcmd\ext\vcvars.bat" arm64
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,win10-cs,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=ARM64 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,win10-cs,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=ARM64 %CUSTOM_PROPS%
 