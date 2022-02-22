@echo off
cd %~dp0
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

call tools\RunMsBuild.bat x64 Debug "sqlite:Rebuild,zlib:Rebuild,sqlite-uwp:Rebuild,win32-dll:Rebuild,win32-lib:Rebuild,net40:Rebuild,win10-cs:Rebuild,win10-dll:Rebuild,Tests\UnitTests:Rebuild,Tests\FuncTests:Rebuild"
call tools\RunTests.bat x64 Debug