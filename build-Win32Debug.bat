@echo off
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

echo "Building using Visual Studio 2017 tools"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=Solutions\AriaSDK.sln

msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,gmock,gtest,UnitTests,FuncTests /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=Debug /p:Platform=Win32
Solutions\out\Debug\Win32\UnitTests\UnitTests.exe
Solutions\out\Debug\Win32\FuncTests\FuncTests.exe
