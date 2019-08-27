@echo off
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

REM Possible platforms: Win32|x64
set PLATFORM=%1
REM Possible configurations: Release|Debug
set CONFIGURATION=%2

echo "Building using Visual Studio 2017 tools"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set SOLUTION=Solutions\MSTelemetrySDK.sln

msbuild %SOLUTION% /target:sqlite,zlib,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=%CONFIGURATION% /p:Platform=%PLATFORM%
if errorLevel 1 goto end
Solutions\out\%CONFIGURATION%\%PLATFORM%\UnitTests\UnitTests.exe
if errorLevel 1 goto end
Solutions\out\%CONFIGURATION%\%PLATFORM%\FuncTests\FuncTests.exe
:end
