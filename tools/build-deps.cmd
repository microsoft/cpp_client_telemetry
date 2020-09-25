@echo off
echo Building 3rd party dependencies...
@setlocal ENABLEEXTENSIONS

set ROOT=%~dp0\..

call %~dp0\vcvars.cmd

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=%ROOT%\Solutions\MSTelemetrySDK.sln


REM Build gmock and gtest
msbuild %SOLUTION% /target:Tests\gmock,Tests\gtest /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=Win32
msbuild %SOLUTION% /target:Tests\gmock,Tests\gtest /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=Win32
msbuild %SOLUTION% /target:Tests\gmock,Tests\gtest /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=x64
msbuild %SOLUTION% /target:Tests\gmock,Tests\gtest /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=x64

REM DLL and static /MD build
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=Win32
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=Win32
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=x64
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=x64

REM Static /MT build
msbuild %SOLUTION% /target:sqlite,zlib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug.vs2015.MT-sqlite /p:Platform=Win32
msbuild %SOLUTION% /target:sqlite,zlib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release.vs2015.MT-sqlite /p:Platform=Win32
msbuild %SOLUTION% /target:sqlite,zlib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug.vs2015.MT-sqlite /p:Platform=x64
msbuild %SOLUTION% /target:sqlite,zlib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release.vs2015.MT-sqlite /p:Platform=x64

REM ARM DLL build
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\vsdevcmd\ext\vcvars.bat" arm
msbuild %SOLUTION% /target:zlib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=ARM
msbuild %SOLUTION% /target:zlib /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=ARM

REM ARM64 DLL build
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\vsdevcmd\ext\vcvars.bat" arm64
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=ARM64
msbuild %SOLUTION% /target:sqlite,zlib,sqlite-uwp /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=ARM64
