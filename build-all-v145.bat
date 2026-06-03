@echo off

set VSTOOLS_VERSION=vs2026
set PlatformToolset=v145
set SKIP_NET40_BUILD=1
call build-all-windows.bat %*
