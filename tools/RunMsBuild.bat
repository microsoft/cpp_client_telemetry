@echo off
@setlocal ENABLEEXTENSIONS

set TARGETPLATFORM=%1
set CONFIGURATION=%2
set TARGETS=%~3

set CUSTOM_PROPS=
if ("%~4" == "") goto endCustomProps
set CUSTOM_PROPS=%4
echo Using custom properties file for the build:
echo %CUSTOM_PROPS%
:endCustomProps

call tools\vcvars.cmd

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=Solutions\MSTelemetrySDK.sln

msbuild %SOLUTION% /target:%TARGETS% /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=%CONFIGURATION% /p:Platform=%TARGETPLATFORM% %CUSTOM_PROPS%