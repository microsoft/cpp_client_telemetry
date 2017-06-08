@echo off
@setlocal ENABLEEXTENSIONS
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsDevCmd.bat"

if /i '%OACR_Path%' == '' (
echo.
echo ERROR! OACR_Path variable must be set to generate OACR files!
echo.
goto :EOF
)

if /i '%ARIAROOT%' == '' (
echo.
echo ERROR! ARIAROOT variable must be set to generate OACR files!
echo.
goto :EOF
)

set Path=%Path%;%OACR_Path%;
set ClToolExe=oacrcl.exe
set CscToolExe=oacrcsc.exe

call oacr clean /all
call oacr stop
call oacr set on

rd /s /q %ARIAROOT%\solutions\out\oacr

echo ***************************************************************************************************
echo ***************************************************************************************************
echo ** Building projects using MSBuild...                                                            **
echo ***************************************************************************************************
echo ***************************************************************************************************

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=

echo ***************************************************************************************************
echo ***************************************************************************************************
echo ** Running OACR on Win32 Release ...                                                             **
echo ***************************************************************************************************
echo ***************************************************************************************************
msbuild sqlite\sqlite.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32 /p:RunOACR=Yes
msbuild sqliteUWP\sqliteUWP.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32 /p:RunOACR=Yes
msbuild Solutions\lib\aria.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32 /p:RunOACR=Yes
msbuild Solutions\win32-dll\win32-dll.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32 /p:RunOACR=Yes
msbuild Solutions\win10-dll\win10-dll.vcxproj /maxcpucount:1 /detailedsummary /p:Configuration=release /p:Platform=Win32 /p:RunOACR=Yes

call oacr showqueue all
call oacr check all
call oacr view /errors
call oacr set off
