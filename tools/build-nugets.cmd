@echo off

cd %~dp0
set PATH=%CD%;%PATH%
cd ..
set ROOT=%CD%
set /p PackageVersion=<%ROOT%\Solutions\version.txt
set OUTDIR=%ROOT%\dist\aria-windows-sdk\%PackageVersion%
set ProjectName=Microsoft.Applications.Telemetry.Windows

cd %ROOT%\tools\NuGet

for %%x in (
  net40
) do (
  cd %%x
  del *.nupkg
  call build.cmd
  cd ..
)

exit
  native
  uap
  net40
  static-vc140-MD
  static-vc140-MT
