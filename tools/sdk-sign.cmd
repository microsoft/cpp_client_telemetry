@echo off
REM ************************
REM * SDK signing script   *
REM ************************

cd %~dp0
set PATH=%CD%;%PATH%
cd ..

set ROOT=%CD%
set /p PackageVersion=<%ROOT%\Solutions\version.txt
set SRCDIR=%ROOT%\Solutions\out
set OUTDIR=%ROOT%\dist\aria-windows-sdk\%PackageVersion%

cd %OUTDIR%

mkdir arm
cd arm
del /s *.dll
cd ..

mkdir intel
cd intel
del /s *.dll
cd ..

copy lib\uap10\ARM\Debug\Microsoft.Applications.Telemetry.Windows-managed.dll			arm\uap10_ARM_Debug_Microsoft.Applications.Telemetry.Windows-managed.dll
copy lib\uap10\ARM\Release\Microsoft.Applications.Telemetry.Windows-managed.dll			arm\uap10_ARM_Release_Microsoft.Applications.Telemetry.Windows-managed.dll
copy lib\uap10\ARM64\Debug\Microsoft.Applications.Telemetry.Windows-managed.dll			arm\uap10_ARM64_Debug_Microsoft.Applications.Telemetry.Windows-managed.dll
copy lib\uap10\ARM64\Release\Microsoft.Applications.Telemetry.Windows-managed.dll		arm\uap10_ARM64_Release_Microsoft.Applications.Telemetry.Windows-managed.dll

copy lib\uap10\x64\Debug\Microsoft.Applications.Telemetry.Windows-managed.dll			intel\uap10_x64_Debug_Microsoft.Applications.Telemetry.Windows-managed.dll
copy lib\uap10\x64\Release\Microsoft.Applications.Telemetry.Windows-managed.dll			intel\uap10_x64_Release_Microsoft.Applications.Telemetry.Windows-managed.dll
copy lib\uap10\x86\Debug\Microsoft.Applications.Telemetry.Windows-managed.dll			intel\uap10_x86_Debug_Microsoft.Applications.Telemetry.Windows-managed.dll
copy lib\uap10\x86\Release\Microsoft.Applications.Telemetry.Windows-managed.dll			intel\uap10_x86_Release_Microsoft.Applications.Telemetry.Windows-managed.dll

copy lib\win10\ARM\Debug\Microsoft.Applications.Telemetry.Windows-native.dll			arm\win10_ARM_Debug_Microsoft.Applications.Telemetry.Windows-native.dll
copy lib\win10\ARM\Release\Microsoft.Applications.Telemetry.Windows-native.dll			arm\win10_ARM_Release_Microsoft.Applications.Telemetry.Windows-native.dll
copy lib\win10\ARM64\Debug\Microsoft.Applications.Telemetry.Windows-native.dll			arm\win10_ARM64_Debug_Microsoft.Applications.Telemetry.Windows-native.dll
copy lib\win10\ARM64\Release\Microsoft.Applications.Telemetry.Windows-native.dll		arm\win10_ARM64_Release_Microsoft.Applications.Telemetry.Windows-native.dll

copy lib\win10\x64\Debug\Microsoft.Applications.Telemetry.Windows-native.dll			intel\win10_x64_Debug_Microsoft.Applications.Telemetry.Windows-native.dll
copy lib\win10\x64\Release\Microsoft.Applications.Telemetry.Windows-native.dll			intel\win10_x64_Release_Microsoft.Applications.Telemetry.Windows-native.dll
copy lib\win10\x86\Debug\Microsoft.Applications.Telemetry.Windows-native.dll			intel\win10_x86_Debug_Microsoft.Applications.Telemetry.Windows-native.dll
copy lib\win10\x86\Release\Microsoft.Applications.Telemetry.Windows-native.dll			intel\win10_x86_Release_Microsoft.Applications.Telemetry.Windows-native.dll

copy lib\win32-dll-vs2015\x64\Debug\ClientTelemetry.dll						intel\win32-dll-vs2015_x64_Debug_ClientTelemetry.dll
copy lib\win32-dll-vs2015\x64\Release\ClientTelemetry.dll					intel\win32-dll-vs2015_x64_Release_ClientTelemetry.dll
copy lib\win32-dll-vs2015\x86\Debug\ClientTelemetry.dll						intel\win32-dll-vs2015_x86_Debug_ClientTelemetry.dll
copy lib\win32-dll-vs2015\x86\Release\ClientTelemetry.dll					intel\win32-dll-vs2015_x86_Release_ClientTelemetry.dll
REM  No ARM 32-bit Win32 Desktop support, only ARM64
copy lib\win32-dll-vs2015\ARM64\Debug\ClientTelemetry.dll					arm\win32-dll-vs2015_ARM64_Debug_ClientTelemetry.dll
copy lib\win32-dll-vs2015\ARM64\Release\ClientTelemetry.dll					arm\win32-dll-vs2015_ARM64_Release_ClientTelemetry.dll

copy lib\win32-net40-vs2015\x64\Debug\Microsoft.Applications.Telemetry.Windows-net40.dll	intel\win32-net40-vs2015_x64_Debug_Microsoft.Applications.Telemetry.Windows-net40.dll
copy lib\win32-net40-vs2015\x64\Release\Microsoft.Applications.Telemetry.Windows-net40.dll	intel\win32-net40-vs2015_x64_Release_Microsoft.Applications.Telemetry.Windows-net40.dll
copy lib\win32-net40-vs2015\x86\Debug\Microsoft.Applications.Telemetry.Windows-net40.dll	intel\win32-net40-vs2015_x86_Debug_Microsoft.Applications.Telemetry.Windows-net40.dll
copy lib\win32-net40-vs2015\x86\Release\Microsoft.Applications.Telemetry.Windows-net40.dll	intel\win32-net40-vs2015_x86_Release_Microsoft.Applications.Telemetry.Windows-net40.dll

REM Sign Intel / PC / Desktop .dlls
cd %OUTDIR%
echo | %ROOT%\tools\codesign\bin\x86\Release\codesign.exe %OUTDIR%\windows-sdk-intel.xml -dir %CD%

REM Sign ARM / Phone .dlls
cd %OUTDIR%
echo | %ROOT%\tools\codesign\bin\x86\Release\codesign.exe %OUTDIR%\windows-sdk-arm.xml -dir %CD%
