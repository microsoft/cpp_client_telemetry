@echo off

set /p PackageVersion=<version.txt
set OUTDIR=%CD%\..\..\dist\aria-windows-sdk
set NativeSDKFolder=%OUTDIR%\%PackageVersion%
set TOPDIR=%CD%\..\..\

cd %NativeSDKFolder%

REM TODO: make sure that we check if source file exists. Also accept %1 (intel) and %2 (arm) dir as parameters, so that we can copy in one pass

xcopy /Y %1\arm\uap10_ARM_Debug_Microsoft.Applications.Telemetry.Windows.dll                  lib\uap10\ARM\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\arm\uap10_ARM_Release_Microsoft.Applications.Telemetry.Windows.dll                lib\uap10\ARM\Release\Microsoft.Applications.Telemetry.Windows.dll			
                                                                                      
xcopy /Y %1\intel\uap10_x64_Debug_Microsoft.Applications.Telemetry.Windows.dll                lib\uap10\x64\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\intel\uap10_x64_Release_Microsoft.Applications.Telemetry.Windows.dll              lib\uap10\x64\Release\Microsoft.Applications.Telemetry.Windows.dll			
                                                                                   
xcopy /Y %1\intel\uap10_x86_Debug_Microsoft.Applications.Telemetry.Windows.dll                lib\uap10\x86\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\intel\uap10_x86_Release_Microsoft.Applications.Telemetry.Windows.dll              lib\uap10\x86\Release\Microsoft.Applications.Telemetry.Windows.dll			
                                                                                   
xcopy /Y %1\arm\win10_ARM_Debug_Microsoft.Applications.Telemetry.Windows.dll                  lib\win10\ARM\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\arm\win10_ARM_Release_Microsoft.Applications.Telemetry.Windows.dll                lib\win10\ARM\Release\Microsoft.Applications.Telemetry.Windows.dll			
                                                                                   
xcopy /Y %1\intel\win10_x64_Debug_Microsoft.Applications.Telemetry.Windows.dll                lib\win10\x64\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\intel\win10_x64_Release_Microsoft.Applications.Telemetry.Windows.dll              lib\win10\x64\Release\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\intel\win10_x86_Debug_Microsoft.Applications.Telemetry.Windows.dll                lib\win10\x86\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\intel\win10_x86_Release_Microsoft.Applications.Telemetry.Windows.dll              lib\win10\x86\Release\Microsoft.Applications.Telemetry.Windows.dll			
                                                                                                                                                                      
xcopy /Y %1\intel\win32-dll-vs2015_x64_Debug_ClientTelemetry.dll                              lib\win32-dll-vs2015\x64\Debug\ClientTelemetry.dll					
xcopy /Y %1\intel\win32-dll-vs2015_x64_Release_ClientTelemetry.dll                            lib\win32-dll-vs2015\x64\Release\ClientTelemetry.dll				
xcopy /Y %1\intel\win32-dll-vs2015_x86_Debug_ClientTelemetry.dll                              lib\win32-dll-vs2015\x86\Debug\ClientTelemetry.dll					
xcopy /Y %1\intel\win32-dll-vs2015_x86_Release_ClientTelemetry.dll                            lib\win32-dll-vs2015\x86\Release\ClientTelemetry.dll				
                                                                                                                                                                      
xcopy /Y %1\intel\win32-net40-vs2015_x64_Debug_Microsoft.Applications.Telemetry.Windows.dll   lib\win32-net40-vs2015\x64\Debug\Microsoft.Applications.Telemetry.Windows.dll	
xcopy /Y %1\intel\win32-net40-vs2015_x64_Release_Microsoft.Applications.Telemetry.Windows.dll lib\win32-net40-vs2015\x64\Release\Microsoft.Applications.Telemetry.Windows.dll	
xcopy /Y %1\intel\win32-net40-vs2015_x86_Debug_Microsoft.Applications.Telemetry.Windows.dll   lib\win32-net40-vs2015\x86\Debug\Microsoft.Applications.Telemetry.Windows.dll	
xcopy /Y %1\intel\win32-net40-vs2015_x86_Release_Microsoft.Applications.Telemetry.Windows.dll lib\win32-net40-vs2015\x86\Release\Microsoft.Applications.Telemetry.Windows.dll	
                                                                                   
xcopy /Y %1\arm\win81_ARM_Debug_Microsoft.Applications.Telemetry.Windows.dll                  lib\win81\ARM\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\arm\win81_ARM_Release_Microsoft.Applications.Telemetry.Windows.dll                lib\win81\ARM\Release\Microsoft.Applications.Telemetry.Windows.dll			
                                                                                   
xcopy /Y %1\intel\win81_x64_Debug_Microsoft.Applications.Telemetry.Windows.dll                lib\win81\x64\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\intel\win81_x64_Release_Microsoft.Applications.Telemetry.Windows.dll              lib\win81\x64\Release\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\intel\win81_x86_Debug_Microsoft.Applications.Telemetry.Windows.dll                lib\win81\x86\Debug\Microsoft.Applications.Telemetry.Windows.dll			
xcopy /Y %1\intel\win81_x86_Release_Microsoft.Applications.Telemetry.Windows.dll              lib\win81\x86\Release\Microsoft.Applications.Telemetry.Windows.dll			
                                                                                   
xcopy /Y %1\arm\winphone81_ARM_Debug_Microsoft.Applications.Telemetry.Windows.dll             lib\winphone81\ARM\Debug\Microsoft.Applications.Telemetry.Windows.dll		
xcopy /Y %1\arm\winphone81_ARM_Release_Microsoft.Applications.Telemetry.Windows.dll           lib\winphone81\ARM\Release\Microsoft.Applications.Telemetry.Windows.dll		
                                                                                   
xcopy /Y %1\intel\winphone81_x86_Debug_Microsoft.Applications.Telemetry.Windows.dll           lib\winphone81\x86\Debug\Microsoft.Applications.Telemetry.Windows.dll		
xcopy /Y %1\intel\winphone81_x86_Release_Microsoft.Applications.Telemetry.Windows.dll         lib\winphone81\x86\Release\Microsoft.Applications.Telemetry.Windows.dll		
