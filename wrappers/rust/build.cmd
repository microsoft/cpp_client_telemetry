@echo off

set VSTOOLS_VERSION=vs2019
cd %~dp0

call ..\..\tools\vcvars.cmd

REM ********************************************************************
REM buildgen + rust
REM ********************************************************************

cargo build