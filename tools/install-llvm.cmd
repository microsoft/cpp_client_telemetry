@echo off
set "PATH=C:\Windows;C:\Windows\System32;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\Program Files\Git\bin"
cd %~dp0

call powershell -File .\install-llvm.ps1

REM Download Visual Studio LLVM extension required for clang build
if NOT exist llvm.vsix (
  call download.cmd https://llvmextensions.gallerycdn.vsassets.io/extensions/llvmextensions/llvm-toolchain/1.0.363769/1560930595399/llvm.vsix
)

if NOT exist "%VSINSTALLDIR%" (
  REM Detect Visual Studio path
  call vcvars.cmd
)

REM Install optional components
set "VSIXInstaller=%VSINSTALLDIR%\Common7\IDE\VSIXInstaller.exe"
if exist "%VSIXInstaller%" (
  "%VSIXInstaller%" /q /a llvm.vsix
)

REM Ignore failures if components have been already installed
EXIT /b 0
