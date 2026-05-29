@echo off

cd %~dp0
@setlocal ENABLEEXTENSIONS

call tools\gen-version.cmd
if errorLevel 1 goto end

git -c submodule."lib/modules".update=none submodule update --init --recursive --depth=1
if errorLevel 1 goto end

call tools\RunMsBuild.bat x64 Release "sqlite:Rebuild,zlib:Rebuild,sqlite-uwp:Rebuild,win32-lib:Rebuild,win10-lib:Rebuild"

:end
exit /b %errorLevel%
