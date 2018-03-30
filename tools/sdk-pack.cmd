@echo off

set /p PackageVersion=<version.txt
set OUTDIR=%CD%\..\..\dist\aria-windows-sdk
set NativeSDKFolder=%OUTDIR%\%PackageVersion%
set TOPDIR=%CD%\..\..\
set PATH=C:\MinGW\msys\1.0\bin;%PATH%

cd %NativeSDKFolder%

zip -r -9 aria-windows-sdk-win32.zip version.txt windows-sdk.md include lib\win32-dll-vs2013 lib\win32-dll-vs2015 lib\win32-lib-vs2013-md lib\win32-lib-vs2013-mt lib\win32-lib-vs2015-md lib\win32-lib-vs2013-mt-sqlite
zip -r -9 aria-windows-sdk-net40.zip version.txt windows-sdk.md include lib\win32-net40-vs2013 lib\win32-net40-vs2015
REM zip -r -9 aria-windows-sdk-winrt.zip version.txt windows-sdk.md include lib\win81 lib\winphone81
zip -r -9 aria-windows-sdk-win10cpp.zip version.txt windows-sdk.md include lib\win10
zip -r -9 aria-windows-sdk-win10uap.zip version.txt windows-sdk.md include lib\uap10
