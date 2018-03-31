@echo off
cd /d %~dp0
set OUTDIR=%CD%\..\..\out
cd %OUTDIR%

xcopy /Y /D Debug\Win32\net40\bin\Microsoft.Applications.Telemetry.Windows.dll				Debug\x86\win32-cs\bin\
xcopy /Y /D Debug\x64\net40\bin\Microsoft.Applications.Telemetry.Windows.dll				Debug\x64\win32-cs\bin\

xcopy /Y /D Debug.vs2013\Win32\net40\bin\Microsoft.Applications.Telemetry.Windows.dll			Debug.vs2013\x86\win32-cs\bin\
xcopy /Y /D Debug.vs2013\x64\net40\bin\Microsoft.Applications.Telemetry.Windows.dll			Debug.vs2013\x64\win32-cs\bin\

xcopy /Y /D Release\Win32\net40\bin\Microsoft.Applications.Telemetry.Windows.dll			Release\x86\win32-cs\bin\
xcopy /Y /D Release\x64\net40\bin\Microsoft.Applications.Telemetry.Windows.dll				Release\x64\win32-cs\bin\

xcopy /Y /D Release.vs2013\Win32\net40\bin\Microsoft.Applications.Telemetry.Windows.dll			Release.vs2013\x86\win32-cs\bin\
xcopy /Y /D Release.vs2013\x64\net40\bin\Microsoft.Applications.Telemetry.Windows.dll			Release.vs2013\x64\win32-cs\bin\

exit /b 0

