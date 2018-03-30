@echo off
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
MSBuild ClientTelemetry.Windows.sln /maxcpucount:1 /detailedsummary

echo ***************************************************************************************************
echo ***************************************************************************************************
echo ** Creating unsigned SDK ...                                                                     **
echo ***************************************************************************************************
echo ***************************************************************************************************
call sdk-create.cmd
