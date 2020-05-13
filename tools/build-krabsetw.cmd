@echo off
echo Building KrabsETW (test only and debug tool dependency)...
@setlocal ENABLEEXTENSIONS

set ROOT=%~dp0\..

echo Using Visual Studio 2019 tools...
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat"

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=%ROOT%\third_party\krabsetw\krabs\krabs.sln

msbuild %SOLUTION% /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Debug /p:Platform=x64
msbuild %SOLUTION% /maxcpucount:%MAXCPUCOUNT% /p:Configuration=Release /p:Platform=x64
