@echo off
REM
REM Build 'compact' Win 10 SDK for UTC mode :
REM * exclude built-in SQLite
REM * keep experimentation client
REM
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

echo "Building using Visual Studio 2017 tools"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=Solutions\MSTelemetrySDK.sln

REM Avoid linking sqlite library into .DLL
set NOSQLITE=TRUE
set CUSTOM_PROPS="/p:ForceImportBeforeCppTargets=%~dp0\Solutions\build.compact-exp.props"
echo Using custom properties file for the build:
echo %CUSTOM_PROPS%

REM DLL and static /MD build
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=x64 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=x64 %CUSTOM_PROPS%

REM ARM DLL build
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\vsdevcmd\ext\vcvars.bat" arm
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=ARM %CUSTOM_PROPS%
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=ARM %CUSTOM_PROPS%

REM ARM64 DLL build
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\vsdevcmd\ext\vcvars.bat" arm64
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=ARM64 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=ARM64 %CUSTOM_PROPS%
 