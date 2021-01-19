@echo off
REM ************************
REM * SDK packaging script *
REM ************************

cd %~dp0
set PATH=%CD%;%PATH%
cd ..

set ROOT=%CD%
set /p PackageVersion=<%ROOT%\Solutions\out\version.txt
set SRCDIR=%ROOT%\Solutions\out
set OUTDIR=%ROOT%\dist\aria-windows-sdk\%PackageVersion%
set ProjectName=Microsoft.Applications.Telemetry.Windows

mkdir %OUTDIR%\include
mkdir %OUTDIR%\lib

echo Copy codesign configs...
copy /Y %ROOT%\tools\*.xml %OUTDIR%\

echo Copy header files...
robocopy %ROOT%\lib\include\public %OUTDIR%\include

echo Copy all binaries...

echo Windows 10 native...
call sku-create.cmd win10                win10-dll

echo Windows 10 managed...
call sku-create.cmd uap10                win10-cs

echo Windows Desktop (win32) .NET 4.x...
call sku-create.cmd win32-net40-vs2015   net40

echo Windows Desktop (win32) .dll...
call sku-create.cmd win32-dll-vs2015     win32-dll

echo Windows Desktop (win32) static runtime .lib...
call sku-create.cmd win32-lib-vs2015-mt-sqlite  win32-lib        vs2015.MT-sqlite
call sku-create.cmd win32-lib-vs2015-mt-sqlite  sqlite           vs2015.MT-sqlite
call sku-create.cmd win32-lib-vs2015-mt-sqlite  zlib             vs2015.MT-sqlite

echo Windows Desktop (win32) dynamic runtime .lib...
call sku-create.cmd win32-lib-vs2015-md  win32-lib
call sku-create.cmd win32-lib-vs2015-md  sqlite
call sku-create.cmd win32-lib-vs2015-md  zlib

echo "Copy Changelog.md"
if exist "%ROOT%\CHANGELOG.md" (
  copy /Y %ROOT%\CHANGELOG.md %OUTDIR%\
)

