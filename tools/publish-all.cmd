set /p PackageVersion=<../version.txt

call publish-nupkg.cmd native\Microsoft.Applications.Telemetry.Windows.%PackageVersion%.nupkg
call publish-nupkg.cmd uap\Microsoft.Applications.Telemetry.Windows.UAP.%PackageVersion%.nupkg
call publish-nupkg.cmd net40\Microsoft.Applications.Telemetry.Desktop.%PackageVersion%.nupkg
call publish-nupkg.cmd static-vc140-MD\Microsoft.Applications.Telemetry.Windows.static-vc140-MD.%PackageVersion%.nupkg
call publish-nupkg.cmd static-vc140-MT\Microsoft.Applications.Telemetry.Windows.static-vc140-MT.%PackageVersion%.nupkg
