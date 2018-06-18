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
msbuild Solutions\AriaSDK.sln /target:build\build-sdk /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=Debug /p:Platform=Win32
msbuild Solutions\AriaSDK.sln /target:build\build-sdk /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=Release /p:Platform=Win32
msbuild Solutions\AriaSDK.sln /target:build\build-sdk /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=Debug /p:Platform=x64
msbuild Solutions\AriaSDK.sln /target:build\build-sdk /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=Release /p:Platform=x64
msbuild Solutions\AriaSDK.sln /target:win10-cs,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=Debug /p:Platform=ARM
msbuild Solutions\AriaSDK.sln /target:win10-cs,win10-dll /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=Release /p:Platform=ARM
 