@echo off
WHERE choco >nul 2>nul
IF %ERRORLEVEL% NEQ 0 (
  echo This script requires chocolatey. Installation instructions: https://chocolatey.org/docs/installation
  exit -1
)

choco install -y cmake
choco install -y svn
choco install -y git
choco install -y llvm
choco install -y zip
REM choco install -y visualstudio2017buildtools
REM choco install -y visualstudio2017enterprise
