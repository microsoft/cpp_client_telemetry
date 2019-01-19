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

xcopy /Y %1\intel\uap10_x64_Debug_Microsoft.Applications.Telemetry.Windows-managed.dll			lib\uap10\x64\Debug\Microsoft.Applications.Telemetry.Windows-managed.dll
xcopy /Y %1\intel\uap10_x64_Release_Microsoft.Applications.Telemetry.Windows-managed.dll		lib\uap10\x64\Release\Microsoft.Applications.Telemetry.Windows-managed.dll
xcopy /Y %1\intel\uap10_x86_Debug_Microsoft.Applications.Telemetry.Windows-managed.dll			lib\uap10\x86\Debug\Microsoft.Applications.Telemetry.Windows-managed.dll
xcopy /Y %1\intel\uap10_x86_Release_Microsoft.Applications.Telemetry.Windows-managed.dll		lib\uap10\x86\Release\Microsoft.Applications.Telemetry.Windows-managed.dll

xcopy /Y %1\intel\win10_x64_Debug_Microsoft.Applications.Telemetry.Windows-native.dll			lib\win10\x64\Debug\Microsoft.Applications.Telemetry.Windows-native.dll
xcopy /Y %1\intel\win10_x64_Release_Microsoft.Applications.Telemetry.Windows-native.dll			lib\win10\x64\Release\Microsoft.Applications.Telemetry.Windows-native.dll
xcopy /Y %1\intel\win10_x86_Debug_Microsoft.Applications.Telemetry.Windows-native.dll			lib\win10\x86\Debug\Microsoft.Applications.Telemetry.Windows-native.dll
xcopy /Y %1\intel\win10_x86_Release_Microsoft.Applications.Telemetry.Windows-native.dll			lib\win10\x86\Release\Microsoft.Applications.Telemetry.Windows-native.dll

xcopy /Y %1\intel\win32-dll-vs2015_x64_Debug_ClientTelemetry.dll					lib\win32-dll-vs2015\x64\Debug\ClientTelemetry.dll
xcopy /Y %1\intel\win32-dll-vs2015_x64_Release_ClientTelemetry.dll					lib\win32-dll-vs2015\x64\Release\ClientTelemetry.dll
xcopy /Y %1\intel\win32-dll-vs2015_x86_Debug_ClientTelemetry.dll					lib\win32-dll-vs2015\x86\Debug\ClientTelemetry.dll
xcopy /Y %1\intel\win32-dll-vs2015_x86_Release_ClientTelemetry.dll					lib\win32-dll-vs2015\x86\Release\ClientTelemetry.dll

xcopy /Y %1\intel\win32-net40-vs2015_x64_Debug_Microsoft.Applications.Telemetry.Windows-net40.dll	lib\win32-net40-vs2015\x64\Debug\Microsoft.Applications.Telemetry.Windows-net40.dll
xcopy /Y %1\intel\win32-net40-vs2015_x64_Release_Microsoft.Applications.Telemetry.Windows-net40.dll	lib\win32-net40-vs2015\x64\Release\Microsoft.Applications.Telemetry.Windows-net40.dll
xcopy /Y %1\intel\win32-net40-vs2015_x86_Debug_Microsoft.Applications.Telemetry.Windows-net40.dll	lib\win32-net40-vs2015\x86\Debug\Microsoft.Applications.Telemetry.Windows-net40.dll
xcopy /Y %1\intel\win32-net40-vs2015_x86_Release_Microsoft.Applications.Telemetry.Windows-net40.dll	lib\win32-net40-vs2015\x86\Release\Microsoft.Applications.Telemetry.Windows-net40.dll

xcopy /Y %2\arm\uap10_ARM_Debug_Microsoft.Applications.Telemetry.Windows-managed.dll			lib\uap10\ARM\Debug\Microsoft.Applications.Telemetry.Windows-managed.dll
xcopy /Y %2\arm\uap10_ARM_Release_Microsoft.Applications.Telemetry.Windows-managed.dll			lib\uap10\ARM\Release\Microsoft.Applications.Telemetry.Windows-managed.dll
xcopy /Y %2\arm\uap10_ARM64_Debug_Microsoft.Applications.Telemetry.Windows-managed.dll			lib\uap10\ARM64\Debug\Microsoft.Applications.Telemetry.Windows-managed.dll
xcopy /Y %2\arm\uap10_ARM64_Release_Microsoft.Applications.Telemetry.Windows-managed.dll		lib\uap10\ARM64\Release\Microsoft.Applications.Telemetry.Windows-managed.dll

xcopy /Y %2\arm\win10_ARM_Debug_Microsoft.Applications.Telemetry.Windows-native.dll			lib\win10\ARM\Debug\Microsoft.Applications.Telemetry.Windows-native.dll
xcopy /Y %2\arm\win10_ARM_Release_Microsoft.Applications.Telemetry.Windows-native.dll			lib\win10\ARM\Release\Microsoft.Applications.Telemetry.Windows-native.dll
xcopy /Y %2\arm\win10_ARM64_Debug_Microsoft.Applications.Telemetry.Windows-native.dll			lib\win10\ARM64\Debug\Microsoft.Applications.Telemetry.Windows-native.dll
xcopy /Y %2\arm\win10_ARM64_Release_Microsoft.Applications.Telemetry.Windows-native.dll			lib\win10\ARM64\Release\Microsoft.Applications.Telemetry.Windows-native.dll

xcopy /Y %2\arm\win32-dll-vs2015_ARM64_Debug_ClientTelemetry.dll					lib\win32-dll-vs2015\ARM64\Debug\ClientTelemetry.dll
xcopy /Y %2\arm\win32-dll-vs2015_ARM64_Release_ClientTelemetry.dll					lib\win32-dll-vs2015\ARM64\Release\ClientTelemetry.dll
