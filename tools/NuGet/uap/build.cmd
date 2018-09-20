@ECHO OFF                                                      	

REM TODO: add Debug flavor to the build

set ProjectName=Microsoft.Applications.Telemetry.Windows
set NuGetFolder=%CD%
set NativeSDKFolder=%OUTDIR%

cd %NuGetFolder%
del /s *.pri *.lib *.winmd *.dll *.obj *.pdb *.exp *.iobj *.ipdb *.bsc
echo Building package for managed code - C#

md %NuGetFolder%\lib\uap10.0
md %NuGetFolder%\lib\uap10.0\x86
md %NuGetFolder%\lib\uap10.0\x64
md %NuGetFolder%\lib\uap10.0\arm
md %NuGetFolder%\lib\uap10.0\arm64

md %NuGetFolder%\lib\win
md %NuGetFolder%\runtimes\win10-x86\native
md %NuGetFolder%\runtimes\win10-x64\native
md %NuGetFolder%\runtimes\win10-arm\native
md %NuGetFolder%\runtimes\win10-arm64\native

xcopy %NativeSDKFolder%\lib\uap10\x86\Release\*.winmd                  %NuGetFolder%\lib\uap10.0\ /Y

REM UAP/X86 release
xcopy %NativeSDKFolder%\lib\uap10\x86\Release\*.pri		       %NuGetFolder%\lib\uap10.0\x86\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x86\Release\*.winmd		       %NuGetFolder%\lib\uap10.0\x86\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x86\Release\*.dll		       %NuGetFolder%\lib\uap10.0\x86\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x86\Release\*.pri		       %NuGetFolder%\lib\win\x86\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x86\Release\*.winmd		       %NuGetFolder%\lib\win\x86\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x86\Release\*.dll		       %NuGetFolder%\lib\win\x86\ /Y

REM UAP/x64 release
xcopy %NativeSDKFolder%\lib\uap10\x64\Release\*.pri		       %NuGetFolder%\lib\uap10.0\x64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x64\Release\*.winmd		       %NuGetFolder%\lib\uap10.0\x64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x64\Release\*.dll		       %NuGetFolder%\lib\uap10.0\x64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x64\Release\*.pri		       %NuGetFolder%\lib\win\x64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x64\Release\*.winmd		       %NuGetFolder%\lib\win\x64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x64\Release\*.dll		       %NuGetFolder%\lib\win\x64\ /Y

REM UAP/ARM release
xcopy %NativeSDKFolder%\lib\uap10\ARM\Release\*.pri		       %NuGetFolder%\lib\uap10.0\arm\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM\Release\*.winmd		       %NuGetFolder%\lib\uap10.0\arm\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM\Release\*.dll		       %NuGetFolder%\lib\uap10.0\arm\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM\Release\*.pri		       %NuGetFolder%\lib\win\arm\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM\Release\*.winmd		       %NuGetFolder%\lib\win\arm\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM\Release\*.dll		       %NuGetFolder%\lib\win\arm\ /Y

REM UAP/ARM64 release
xcopy %NativeSDKFolder%\lib\uap10\ARM64\Release\*.pri		       %NuGetFolder%\lib\uap10.0\arm64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM64\Release\*.winmd		       %NuGetFolder%\lib\uap10.0\arm64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM64\Release\*.dll		       %NuGetFolder%\lib\uap10.0\arm64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM64\Release\*.pri		       %NuGetFolder%\lib\win\arm64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM64\Release\*.winmd		       %NuGetFolder%\lib\win\arm64\ /Y
xcopy %NativeSDKFolder%\lib\uap10\ARM64\Release\*.dll		       %NuGetFolder%\lib\win\arm64\ /Y

REM copy modules to runtimes directory
xcopy %NativeSDKFolder%\lib\uap10\x86\Release\*.dll		       %NuGetFolder%\runtimes\win10-x86\native\ /Y
xcopy %NativeSDKFolder%\lib\uap10\x64\Release\*.dll		       %NuGetFolder%\runtimes\win10-x64\native\ /Y
xcopy %NativeSDKFolder%\lib\uap10\arm\Release\*.dll                    %NuGetFolder%\runtimes\win10-arm\native\ /Y
xcopy %NativeSDKFolder%\lib\uap10\arm64\Release\*.dll                  %NuGetFolder%\runtimes\win10-arm64\native\ /Y

NuGet.exe pack %NuGetFolder%\%ProjectName%.UAP.nuspec -Version %PackageVersion%

REM Cleanup
REM del /s *.pri *.lib *.winmd *.dll *.obj *.pdb *.exp *.iobj *.ipdb *.bsc
