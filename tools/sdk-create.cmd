@echo off

set /p PackageVersion=<version.txt
set OUTDIR=%CD%\..\..\dist\aria-windows-sdk
set NativeSDKFolder=%OUTDIR%\%PackageVersion%
set ProjectName=Microsoft.Applications.Telemetry.Windows

REM Clean-up
set LASTDIR=%CD%
mkdir %NativeSDKFolder%
mkdir %NativeSDKFolder%\include
cd %NativeSDKFolder%
del /s *.dll
del /s *.lib
del /s *.pdb

cd %LASTDIR%

echo Copy header files...
xcopy ..\include\*.hpp "%NativeSDKFolder%\include" /Y /I

echo Copy all binaries...

REM Windows 10 nuget packages
call sku-create.cmd win10                win10-dll
call sku-create.cmd uap10                win10-cs

REM Windows 8.1 .zip archive
REM call sku-create.cmd win81                win81-cs
REM call sku-create.cmd winphone81           win81-cs-phone

REM Windows Desktop (win32) .NET 4.x archive
REM call sku-create.cmd win32-net40-vs2013   net40            vs2013
call sku-create.cmd win32-net40-vs2015   net40

REM Windows Desktop (win32) native archive
REM Dynamic .dll
REM call sku-create.cmd win32-dll-vs2013     win32-dll        vs2013
call sku-create.cmd win32-dll-vs2015     win32-dll

REM Static vs2013 .lib
REM call sku-create.cmd win32-lib-vs2013-md  sqlite           vs2013
REM call sku-create.cmd win32-lib-vs2013-md  win32-static     vs2013

REM Small Fifo-based no-SQLite SDK for vs2013.MT
REM call sku-create.cmd win32-lib-vs2013-mt  win32-static     vs2013.MT

REM Big SQLite-based SDK for vs2013.MT
REM call sku-create.cmd win32-lib-vs2013-mt-sqlite  win32-static     vs2013.MT-sqlite
REM call sku-create.cmd win32-lib-vs2013-mt-sqlite  sqlite           vs2013.MT

call sku-create.cmd win32-lib-vs2015-mt-sqlite  win32-static     vs2015.MT-sqlite
call sku-create.cmd win32-lib-vs2015-mt-sqlite  sqlite           vs2015.MT-sqlite
call sku-create.cmd win32-lib-vs2015-mt-sqlite  zlib             vs2015.MT-sqlite

REM vs2013 dynamic SKU
call sku-create.cmd win32-lib-vs2015-md  sqlite
call sku-create.cmd win32-lib-vs2015-md  zlib
call sku-create.cmd win32-lib-vs2015-md  win32-static

copy windows-sdk*.* %NativeSDKFolder%
copy version.txt %NativeSDKFolder%

cd %NativeSDKFolder%
del /s *.bsc *.ilk
