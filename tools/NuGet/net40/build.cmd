@ECHO OFF
set ProjectName=Microsoft.Applications.Telemetry.Desktop
set SRC=win32-net40-vs2015
set DST=net40

robocopy %OUTDIR%\include                include
robocopy %OUTDIR%\lib\%SRC%\x86\Release  lib\%DST%\Release\x86
robocopy %OUTDIR%\lib\%SRC%\x86\Debug    lib\%DST%\Debug\x86
robocopy %OUTDIR%\lib\%SRC%\x64\Release  lib\%DST%\Release\x64
robocopy %OUTDIR%\lib\%SRC%\x64\Debug    lib\%DST%\Debug\x64

NuGet.exe pack %ProjectName%.nuspec -Version %PackageVersion%
