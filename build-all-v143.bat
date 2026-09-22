@echo off

set VSTOOLS_VERSION=vs2022
set PlatformToolset=v143
call "%~dp0build-all-windows.bat" %*
