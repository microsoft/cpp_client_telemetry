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
  native
  uap
) do (
  cd %%x
  del *.nupkg
  call build.cmd
  cd ..
)

REM TODO: add nuget generation for static flavors
REM static-vc140-MD
REM static-vc140-MT
