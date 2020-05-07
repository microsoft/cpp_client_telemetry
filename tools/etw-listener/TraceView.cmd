@ECHO OFF
REM Note that this build script currently requires Visual Studio 2019 Enterprise.
REM Please adjust the path to csc.exe depending on what version is installed.
set "PATH=%~dp0\..;%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\Roslyn;%PATH%"
nuget install Microsoft.Diagnostics.Tracing.TraceEvent
csc.exe /reference:Microsoft.Diagnostics.Tracing.TraceEvent TraceView.cs
          