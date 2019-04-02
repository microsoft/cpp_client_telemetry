set PATH=..\packages\NuGet.CommandLine.3.4.3\tools;%PATH%
nuget sources remove -name "ARIA-SDK"
nuget sources add -name "ARIA-SDK" -source https://msasg.pkgs.visualstudio.com/DefaultCollection/_packaging/ARIA-SDK/nuget/v3/index.json -username %USERNAME% -password %1
