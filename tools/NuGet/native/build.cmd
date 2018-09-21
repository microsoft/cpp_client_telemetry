@ECHO OFF
set ProjectName=Microsoft.Applications.Telemetry.Windows

REM Win 10 Universal native SDK
set DST=uap10.0
robocopy %OUTDIR%\include               .\include
robocopy %OUTDIR%\lib\win10\ARM\Release	.\lib\%DST%\Release\ARM
robocopy %OUTDIR%\lib\win10\ARM\Debug	.\lib\%DST%\Debug\ARM
robocopy %OUTDIR%\lib\win10\ARM64\Release .\lib\%DST%\Release\ARM64
robocopy %OUTDIR%\lib\win10\ARM64\Debug	.\lib\%DST%\Debug\ARM64
robocopy %OUTDIR%\lib\win10\x86\Release	.\lib\%DST%\Release\x86
robocopy %OUTDIR%\lib\win10\x86\Debug	.\lib\%DST%\Debug\x86
robocopy %OUTDIR%\lib\win10\x86\Release	.\lib\%DST%\Release\chpe
robocopy %OUTDIR%\lib\win10\x86\Debug	.\lib\%DST%\Debug\chpe
robocopy %OUTDIR%\lib\win10\x64\Release	.\lib\%DST%\Release\x64
robocopy %OUTDIR%\lib\win10\x64\Debug	.\lib\%DST%\Debug\x64

REM Win32 Desktop native SDK
robocopy %OUTDIR%\lib\win32-dll-vs2015		.\lib\native\win32-dll-vs2015		/E
robocopy .\lib\native\win32-dll-vs2015\x86	.\lib\native\win32-dll-vs2015\chpe	/E

NuGet.exe pack %ProjectName%.nuspec -Version %PackageVersion%
