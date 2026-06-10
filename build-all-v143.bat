@echo off

set VSTOOLS_VERSION=vs2022
set PlatformToolset=v143
set SKIP_NET40_BUILD=1
call "%~dp0build-all-windows.bat" %*
