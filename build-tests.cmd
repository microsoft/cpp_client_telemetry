@echo off
cd %~dp0
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

if DEFINED GIT_PULL_TOKEN (
  rd /s /q lib\modules
  git clone https://%GIT_PULL_TOKEN%:x-oauth-basic@github.com/microsoft/cpp_client_telemetry_modules.git lib\modules
)

set PLATFORM=

REM Possible platforms: Win32|x64
set PLAT=%1
REM Possible configurations: Release|Debug
set CONFIGURATION=%2

set CUSTOM_PROPS=
if ("%3"=="") goto skip
set CUSTOM_PROPS="/p:ForceImportBeforeCppTargets=%3"
echo Using custom properties file for the build:
echo %CUSTOM_PROPS%
:skip

REM Add path to vs2017 MSBuild.exe
set "PATH=%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\"

REM Try to setup vs2017 Dev environment if possible
echo Building using Visual Studio 2017 tools
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set SOLUTION=Solutions\MSTelemetrySDK.sln

msbuild %SOLUTION% /target:sqlite,zlib,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=%CONFIGURATION% /p:Platform=%PLAT% %CUSTOM_PROPS%
if errorLevel 1 goto end
Solutions\out\%CONFIGURATION%\%PLAT%\UnitTests\UnitTests.exe
if errorLevel 1 goto end
Solutions\out\%CONFIGURATION%\%PLAT%\FuncTests\FuncTests.exe
:end
if errorLevel 1 goto end
start "" Solutions\out\%CONFIGURATION%\%PLAT%\FuncTests\FuncTests.exe --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager
start "" Solutions\out\%CONFIGURATION%\%PLAT%\FuncTests\FuncTests.exe --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager
:end
