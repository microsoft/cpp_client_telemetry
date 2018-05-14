@ECHO OFF
set ProjectName=Microsoft.Applications.Telemetry.Windows.static-vc140-MT
set NuGetFolder=%CD%
cd ../..
set SolutionDir=%CD%

set /p PackageVersion=<version.txt
REM This path is managed by Visual Studio nuget package NuGet.CommandLine
REM Make sure it's installed before running this batch file!

set OUTDIR=%CD%\..\..\dist\aria-windows-sdk
set NativeSDKFolder=%OUTDIR%\%PackageVersion%

cd %NuGetFolder%
del /s *.pri *.lib *.winmd *.dll *.obj *.pdb *.exp *.iobj *.ipdb *.bsc

md include
xcopy %NativeSDKFolder%\include\*.*                   	%NuGetFolder%\include\ /Y

md lib\include
robocopy %NativeSDKFolder%\lib\win32-lib-vs2015-mt-sqlite %NuGetFolder%\lib\native\win32-lib-vs2015-mt-sqlite /E

del /s *.tmp *.ilk *.dump *.bsc

set PATH=%CD%;%PATH%
cd %NuGetFolder%
NuGet.exe pack %ProjectName%.nuspec -Version %PackageVersion%
