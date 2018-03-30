@echo off

set /p PackageVersion=<version.txt
set OUTDIR=%CD%\..\..\dist\aria-windows-sdk
set NativeSDKFolder=%OUTDIR%\%PackageVersion%
set TOPDIR=%CD%\..\..\
set PATH=%TOPDIR%\buildscript\clienttelemetry;C:\cygwin\bin

cd %NativeSDKFolder%\lib
REM echo Counting unsigned libs (expected to be zero)...
REM sigcheck -e -h -i -q -s . | grep Unsigned | wc -l
sigcheck -e -h -i -q -s .
echo [ DONE ]

