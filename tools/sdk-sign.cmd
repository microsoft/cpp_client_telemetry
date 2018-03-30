@echo off

set /p PackageVersion=<version.txt
set OUTDIR=%CD%\..\..\dist\aria-windows-sdk
set NativeSDKFolder=%OUTDIR%\%PackageVersion%
set TOPDIR=%CD%\..\..\

cd %NativeSDKFolder%

mkdir arm
cd arm
del /s *.dll
cd ..

mkdir intel
cd intel
del /s *.dll
cd ..

copy lib\uap10\ARM\Debug\Microsoft.Applications.Telemetry.Windows.dll			arm\uap10_ARM_Debug_Microsoft.Applications.Telemetry.Windows.dll
copy lib\uap10\ARM\Release\Microsoft.Applications.Telemetry.Windows.dll			arm\uap10_ARM_Release_Microsoft.Applications.Telemetry.Windows.dll

copy lib\uap10\x64\Debug\Microsoft.Applications.Telemetry.Windows.dll			intel\uap10_x64_Debug_Microsoft.Applications.Telemetry.Windows.dll
copy lib\uap10\x64\Release\Microsoft.Applications.Telemetry.Windows.dll			intel\uap10_x64_Release_Microsoft.Applications.Telemetry.Windows.dll

copy lib\uap10\x86\Debug\Microsoft.Applications.Telemetry.Windows.dll			intel\uap10_x86_Debug_Microsoft.Applications.Telemetry.Windows.dll
copy lib\uap10\x86\Release\Microsoft.Applications.Telemetry.Windows.dll			intel\uap10_x86_Release_Microsoft.Applications.Telemetry.Windows.dll

copy lib\win10\ARM\Debug\Microsoft.Applications.Telemetry.Windows.dll			arm\win10_ARM_Debug_Microsoft.Applications.Telemetry.Windows.dll
copy lib\win10\ARM\Release\Microsoft.Applications.Telemetry.Windows.dll			arm\win10_ARM_Release_Microsoft.Applications.Telemetry.Windows.dll

copy lib\win10\x64\Debug\Microsoft.Applications.Telemetry.Windows.dll			intel\win10_x64_Debug_Microsoft.Applications.Telemetry.Windows.dll
copy lib\win10\x64\Release\Microsoft.Applications.Telemetry.Windows.dll			intel\win10_x64_Release_Microsoft.Applications.Telemetry.Windows.dll
copy lib\win10\x86\Debug\Microsoft.Applications.Telemetry.Windows.dll			intel\win10_x86_Debug_Microsoft.Applications.Telemetry.Windows.dll
copy lib\win10\x86\Release\Microsoft.Applications.Telemetry.Windows.dll			intel\win10_x86_Release_Microsoft.Applications.Telemetry.Windows.dll

copy lib\win32-dll-vs2015\x64\Debug\ClientTelemetry.dll					intel\win32-dll-vs2015_x64_Debug_ClientTelemetry.dll
copy lib\win32-dll-vs2015\x64\Release\ClientTelemetry.dll				intel\win32-dll-vs2015_x64_Release_ClientTelemetry.dll
copy lib\win32-dll-vs2015\x86\Debug\ClientTelemetry.dll					intel\win32-dll-vs2015_x86_Debug_ClientTelemetry.dll
copy lib\win32-dll-vs2015\x86\Release\ClientTelemetry.dll				intel\win32-dll-vs2015_x86_Release_ClientTelemetry.dll

copy lib\win32-net40-vs2015\x64\Debug\Microsoft.Applications.Telemetry.Windows.dll	intel\win32-net40-vs2015_x64_Debug_Microsoft.Applications.Telemetry.Windows.dll
copy lib\win32-net40-vs2015\x64\Release\Microsoft.Applications.Telemetry.Windows.dll	intel\win32-net40-vs2015_x64_Release_Microsoft.Applications.Telemetry.Windows.dll
copy lib\win32-net40-vs2015\x86\Debug\Microsoft.Applications.Telemetry.Windows.dll	intel\win32-net40-vs2015_x86_Debug_Microsoft.Applications.Telemetry.Windows.dll
copy lib\win32-net40-vs2015\x86\Release\Microsoft.Applications.Telemetry.Windows.dll	intel\win32-net40-vs2015_x86_Release_Microsoft.Applications.Telemetry.Windows.dll


REM Sign Intel / PC / Desktop .dlls
cd %NativeSDKFolder%
REM runas /profile /user:REDMOND\actlab /savecred "%TOPDIR%\buildscript\codesign\bin\x86\Release\codesign.exe %NativeSDKFolder%\windows-sdk-intel.xml -dir %CD%"
%TOPDIR%\buildscript\codesign\bin\x86\Release\codesign.exe %NativeSDKFolder%\windows-sdk-intel.xml -dir %CD%

REM Sign ARM / Phone .dlls
cd %NativeSDKFolder%
REM runas /profile /user:REDMOND\actlab /savecred "%TOPDIR%\buildscript\codesign\bin\x86\Release\codesign.exe %NativeSDKFolder%\windows-sdk-arm.xml -dir %CD%"
%TOPDIR%\buildscript\codesign\bin\x86\Release\codesign.exe %NativeSDKFolder%\windows-sdk-arm.xml -dir %CD%
