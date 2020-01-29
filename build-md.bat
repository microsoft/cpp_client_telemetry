@echo off
cd %~dp0
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

if DEFINED GIT_PULL_TOKEN (
  rd /s /q lib\modules
  git clone https://%GIT_PULL_TOKEN%:x-oauth-basic@github.com/microsoft/cpp_client_telemetry_modules.git lib\modules
)

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
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=Win32 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=x64 %CUSTOM_PROPS%
msbuild %SOLUTION% /target:sqlite,zlib,win32-lib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=x64 %CUSTOM_PROPS%
