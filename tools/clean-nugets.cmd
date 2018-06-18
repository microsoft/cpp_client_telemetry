REM Clean all nuget local caches - helps to debug changing nugets locally...
rd /s /q %USERPROFILE%\.nuget\packages\Microsoft.Applications.Telemetry.Windows.UAP\
rd /s /q %USERPROFILE%\.nuget\packages\Microsoft.Applications.Telemetry.Windows\
rd /s /q %USERPROFILE%\.nuget\packages\Microsoft.Applications.Telemetry.Windows.static-vc120-MD\
rd /s /q %USERPROFILE%\.nuget\packages\Microsoft.Applications.Telemetry.Windows.static-vc120-MT\
rd /s /q %USERPROFILE%\.nuget\packages\Microsoft.Applications.Telemetry.Windows.static-vc140-MD\
rd /s /q %USERPROFILE%\.nuget\packages\Microsoft.Applications.Telemetry.Windows.static-vc140-MT\
rd /s /q %USERPROFILE%\.nuget\packages\Microsoft.Applications.Telemetry.Desktop\
rd /s /q %USERPROFILE%\AppData\Local\nuget\
