@echo off
cd %~dp0
REM
REM Build 'compact' Win 10 SDK for UTC mode :
REM * exclude built-in SQLite
REM * keep experimentation client
REM
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

echo Update all public submodules...
git -c submodule."lib/modules".update=none submodule update --init --recursive

if DEFINED GIT_PULL_TOKEN (
  rd /s /q lib\modules
  git clone https://%GIT_PULL_TOKEN%:x-oauth-basic@github.com/microsoft/cpp_client_telemetry_modules.git lib\modules
)

call tools\vcvars.cmd

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
call tools\vcvars-ext.cmd arm
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=ARM %CUSTOM_PROPS%
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=ARM %CUSTOM_PROPS%

REM ARM64 DLL build
call tools\vcvars-ext.cmd arm64
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=ARM64 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:zlib,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=ARM64 %CUSTOM_PROPS%
