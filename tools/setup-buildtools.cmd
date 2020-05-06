@ECHO off
PUSHD "%~dp0"
CD /d "%~dp0"

REM Make sure chocolatey package manager is installed
WHERE choco >NUL 2>NUL
IF %ERRORLEVEL% NEQ 0 call setup-choco.cmd
SET "PATH=%~dp0;%ProgramData%\chocolatey;%ALLUSERSPROFILE%\chocolatey\bin;%PATH%"

REM Install core build tools using chocolatey
choco install -y cmake svn git llvm zip

REM Future workloads to consider :
REM choco install -y visualstudio2017buildtools
REM choco install -y visualstudio2017enterprise
REM choco install -y --includeOptional visualstudio2017-workload-netcoretools
REM choco install -y windows-sdk-10-version-1809-all
REM choco install -y windows-sdk-10.1
REM choco install -y windows-sdk-7.1

REM Install nuget for packaging
IF NOT EXIST nuget.exe (
  call download.cmd "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
)

REM Install Python 3.7 version needed for EMCC
IF NOT EXIST Python3.7 (
  MKDIR "%CD%\Python3.7"
  REM Install Python needed for Emscripten SDK and others
  call download.cmd "https://www.python.org/ftp/python/3.7.4/python-3.7.4-embed-amd64.zip"
  powershell -command "Expand-Archive -LiteralPath python-3.7.4-embed-amd64.zip -DestinationPath Python3.7"
)
