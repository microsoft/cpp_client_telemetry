@echo off
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
echo ** Creating Win32 Debug ...                                                                     **
echo ***************************************************************************************************
echo ***************************************************************************************************
msbuild sqlite\sqlite.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32
msbuild sqliteUWP\sqliteUWP.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32
msbuild Solutions\lib\aria.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32
msbuild Solutions\win32-dll\win32-dll.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32
msbuild Solutions\win10-dll\win10-dll.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32
msbuild Solutions\tests\functests\FuncTests.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32
msbuild Solutions\tests\unittests\UnitTests.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32


Solutions\out\Release\Win32\UnitTests.exe
Solutions\out\Release\Win32\FuncTests.exe

