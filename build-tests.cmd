@echo off
cd %~dp0
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

if DEFINED GIT_PULL_TOKEN (
  rd /s /q lib\modules
  git clone https://%GIT_PULL_TOKEN%:x-oauth-basic@github.com/microsoft/cpp_client_telemetry_modules.git lib\modules
)

set GTEST_PATH=third_party\googletest
if NOT EXIST %GTEST_PATH%\CMakeLists.txt (
  git clone --depth 1 --branch release-1.12.0 https://github.com/google/googletest %GTEST_PATH%
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

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set SOLUTION=Solutions\MSTelemetrySDK.sln

msbuild %SOLUTION% /target:sqlite:Rebuild,zlib:Rebuild,Tests\gmock:Rebuild,Tests\gtest:Rebuild,Tests\UnitTests:Rebuild,Tests\FuncTests:Rebuild /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=%CONFIGURATION% /p:Platform=%PLAT% %CUSTOM_PROPS%
if errorLevel 1 goto end
Solutions\out\%CONFIGURATION%\%PLAT%\UnitTests\UnitTests.exe
if errorLevel 1 goto end
Solutions\out\%CONFIGURATION%\%PLAT%\FuncTests\FuncTests.exe
:end
if errorLevel 1 goto end
start "" Solutions\out\%CONFIGURATION%\%PLAT%\FuncTests\FuncTests.exe --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager
start "" Solutions\out\%CONFIGURATION%\%PLAT%\FuncTests\FuncTests.exe --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager
:end
