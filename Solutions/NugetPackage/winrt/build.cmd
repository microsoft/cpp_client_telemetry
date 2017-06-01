@ECHO OFF

set ProjectName=Microsoft.Applications.Telemetry.Windows
set NuGetFolder=%CD%

cd ../..
set SolutionDir=%CD%

set /p PackageVersion=<version.txt
REM This path is managed by Visual Studio nuget package NuGet.CommandLine
REM Make sure it's installed before running this batch file!
set PATH=%CD%\packages\NuGet.CommandLine.3.4.3\tools;%PATH%

cd %NuGetFolder%
del /s *.pri *.lib *.winmd *.dll *.obj *.pdb *.exp *.iobj *.ipdb *.bsc
echo Building package for Windows 8.1

xcopy %SolutionDir%\out\Release\Win32\win81-cs\bin\*.*       %NuGetFolder%\lib\win81\Release\x86\ /Y
xcopy %SolutionDir%\out\Debug\Win32\win81-cs\bin\*.*         %NuGetFolder%\lib\win81\Debug\x86\   /Y

xcopy %SolutionDir%\out\Release\x64\win81-cs\bin\*.*         %NuGetFolder%\lib\win81\Release\x64\ /Y
xcopy %SolutionDir%\out\Debug\x64\win81-cs\bin\*.*           %NuGetFolder%\lib\win81\Debug\x64\   /Y

xcopy %SolutionDir%\out\Release\ARM\win81-cs-phone\bin\*.*   %NuGetFolder%\lib\wp81\Release\ARM\ /Y
xcopy %SolutionDir%\out\Debug\ARM\win81-cs-phone\bin\*.*     %NuGetFolder%\lib\wp81\Debug\ARM\   /Y

del /s *.obj *.iobj *.ipdb *.bsc

NuGet.exe pack %NuGetFolder%\%ProjectName%.WINRT.nuspec -Version %PackageVersion%

REM Cleanup
REM del /s *.pri *.lib *.winmd *.dll *.obj *.pdb *.exp *.iobj *.ipdb *.bsc
