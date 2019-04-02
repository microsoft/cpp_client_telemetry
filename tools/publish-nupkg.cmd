@echo off
REM set PATH=..\packages\NuGet.CommandLine.3.4.3\tools;%PATH%
NuGet.exe push %1 -Source https://msasg.pkgs.visualstudio.com/DefaultCollection/_packaging/ARIA-SDK/nuget/v3/index.json -ApiKey "ARIA-SDK"
