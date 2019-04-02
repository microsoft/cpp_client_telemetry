@echo off
REM ************************
REM * SDK packaging script *
REM ************************

cd %~dp0
set PATH=%CD%;%PATH%
cd ..

set ROOT=%CD%
set /p PackageVersion=<%ROOT%\Solutions\version.txt
set SRCDIR=%ROOT%\Solutions\out
set OUTDIR=%ROOT%\dist\aria-windows-sdk\%PackageVersion%
set ProjectName=Microsoft.Applications.Telemetry.Windows

set PATH=C:\MinGW\msys\1.0\bin;C:\msys64\usr\bin;%PATH%

cd %OUTDIR%
copy /Y %ROOT%\Solutions\version.txt %OUTDIR%\version.txt

zip -r -9 aria-windows-sdk-win32.zip version.txt release-notes.md include lib\win32-dll-vs2015 lib\win32-lib-vs2015-md lib\win32-lib-vs2015-mt-sqlite
zip -r -9 aria-windows-sdk-net40.zip version.txt release-notes.md include lib\win32-net40-vs2015
zip -r -9 aria-windows-sdk-win10cpp.zip version.txt release-notes.md include lib\win10
zip -r -9 aria-windows-sdk-win10uap.zip version.txt release-notes.md include lib\uap10
