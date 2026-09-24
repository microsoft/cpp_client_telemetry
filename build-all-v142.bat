@echo off

set VSTOOLS_VERSION=vs2019
set PlatformToolset=v142
call "%~dp0build-all-windows.bat" %*
