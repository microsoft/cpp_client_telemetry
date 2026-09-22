@echo off

set VSTOOLS_VERSION=vs2026
set PlatformToolset=v145
call "%~dp0build-all-windows.bat" %*
