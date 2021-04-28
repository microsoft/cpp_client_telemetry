@echo off
@setlocal ENABLEEXTENSIONS

set TARGETPLATFORM=%1
set CONFIGURATION=%2
set TARGETS=%~3

call tools\vcvars.cmd

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=Solutions\MSTelemetrySDK.sln

msbuild %SOLUTION% /target:%TARGETS% /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=%CONFIGURATION% /p:Platform=%TARGETPLATFORM%