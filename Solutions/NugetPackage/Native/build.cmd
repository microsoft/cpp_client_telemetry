@ECHO OFF
set ProjectName=Microsoft.Applications.Telemetry.Windows
set NuGetFolder=%CD%
set LibFolder=%CD%\..\..\..\lib

REM set /p PackageVersion=<version.txt
REM This path is managed by Visual Studio nuget package NuGet.CommandLine
REM Make sure it's installed before running this batch file!
set PATH=%CD%\..\NuGet.CommandLine.3.4.3\tools;%PATH%

set OUTDIR=%CD%\..\..\out
set NativeSDKFolder=%OUTDIR%\native

//cd %NuGetFolder%
//del /s *.pri *.lib *.winmd *.dll *.obj *.pdb *.exp *.iobj *.ipdb *.bsc

REM Windows 10 SDK for C/C++ - .lib / .h no-.winmd linkage
set DST=uap10.0

md %NativeSDKFolder%=

xcopy %NuGetFolder%\AriaSDK.nuspec                      %NativeSDKFolder%\  /Y

md %NativeSDKFolder%\include\api
md %NativeSDKFolder%\include\aria
md %NativeSDKFolder%\include\bond
md %NativeSDKFolder%\include\bond\generated
md %NativeSDKFolder%\include\config
md %NativeSDKFolder%\include\decorators
md %NativeSDKFolder%\include\pal
md %NativeSDKFolder%\include\tpm
md %NativeSDKFolder%\include\utils
md %NativeSDKFolder%\content\Debug32
md %NativeSDKFolder%\content\Debug64
md %NativeSDKFolder%\content\Release32
md %NativeSDKFolder%\content\Release64


xcopy %LibFolder%\api\*.hpp                      	%NativeSDKFolder%\include\api\ /Y
xcopy %LibFolder%\include\aria\*.hpp                    %NativeSDKFolder%\include\aria\ /Y
xcopy %LibFolder%\bond\*.hpp                      	%NativeSDKFolder%\include\bond\ /Y
xcopy %LibFolder%\bond\generated\*.hpp                  %NativeSDKFolder%\include\bond\generated\ /Y
xcopy %LibFolder%\decorators\*.hpp                      %NativeSDKFolder%\include\decorators\ /Y
xcopy %LibFolder%\pal\*.hpp                      	%NativeSDKFolder%\include\pal\ /Y
xcopy %LibFolder%\tpm\*.hpp                      	%NativeSDKFolder%\include\tpm\ /Y
xcopy %LibFolder%\utils\*.hpp                      	%NativeSDKFolder%\include\utils\ /Y


xcopy %OUTDIR%\Debug\Win32\Clie*.*                  	%NativeSDKFolder%\content\Debug32\ /Y
xcopy %OUTDIR%\Debug\Win32\Ari*.*                  	%NativeSDKFolder%\content\Debug32\ /Y
xcopy %OUTDIR%\Debug\x64\Clie*.*           		%NativeSDKFolder%\content\Debug64\ /Y
xcopy %OUTDIR%\Debug\x64\Ari*.*           		%NativeSDKFolder%\content\Debug64\ /Y
xcopy %OUTDIR%\Release\Win32\Clie*.*                  	%NativeSDKFolder%\content\Release32\ /Y
xcopy %OUTDIR%\Release\Win32\Ari*.*                  	%NativeSDKFolder%\content\Release32\ /Y
xcopy %OUTDIR%\release\x64\Clie*.*           		%NativeSDKFolder%\content\Release64\ /Y
xcopy %OUTDIR%\release\x64\Ari*.*           		%NativeSDKFolder%\content\Release64\ /Y

 
set PATH=%CD%;%PATH%
cd %NativeSDKFolder%
NuGet.exe pack AriaSDK.nuspec

cd %NuGetFolder%