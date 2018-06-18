@echo off
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsDevCmd.bat"

echo ***************************************************************************************************
echo ***************************************************************************************************
echo ** Building projects using MSBuild...                                                            **
echo ***************************************************************************************************
echo ***************************************************************************************************

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=

REM Disable parallel builds for now because of a vs2013 bug that randomly triggers :
REM cl : Command line error D8040: error creating or communicating with child process
REM Start in clienttelemetry

echo ***************************************************************************************************
echo ***************************************************************************************************
echo ** Creating x64 Debug ...                                                                        **
echo ***************************************************************************************************
echo ***************************************************************************************************
msbuild Solutions\AriaSDK.sln /target:build\build-sdk /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=Debug /p:Platform=x64
Solutions\out\Debug\x64\UnitTests\UnitTests.exe
Solutions\out\Debug\x64\FuncTests\FuncTests.exe
