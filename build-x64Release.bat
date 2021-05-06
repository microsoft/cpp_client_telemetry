@echo off
cd %~dp0
call tools\gen-version.cmd
@setlocal ENABLEEXTENSIONS

call tools\RunMsBuild.bat x64 Release "sqlite,zlib,sqlite-uwp,win32-dll,win32-lib,net40,win10-cs,win10-dll,Tests\gmock,Tests\gtest,Tests\UnitTests,Tests\FuncTests"
call tools\RunTests.bat x64 Release