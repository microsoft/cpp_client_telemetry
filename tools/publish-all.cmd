cd %~dp0
set PATH=%CD%;%PATH%
cd ..
set ROOT=%CD%
set /p PackageVersion=<%ROOT%\Solutions\version.txt

cd %ROOT%\tools

call publish-nupkg.cmd NuGet\native\Microsoft.Applications.Telemetry.Windows.%PackageVersion%.nupkg
call publish-nupkg.cmd NuGet\net40\Microsoft.Applications.Telemetry.Desktop.%PackageVersion%.nupkg
call publish-nupkg.cmd NuGet\uap\Microsoft.Applications.Telemetry.Windows.UAP.%PackageVersion%.nupkg

REM call publish-nupkg.cmd NuGet\static-vc140-MD\Microsoft.Applications.Telemetry.Windows.static-vc140-MD.%PackageVersion%.nupkg
REM call publish-nupkg.cmd NuGet\static-vc140-MT\Microsoft.Applications.Telemetry.Windows.static-vc140-MT.%PackageVersion%.nupkg
