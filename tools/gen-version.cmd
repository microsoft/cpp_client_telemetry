@echo off
set MYCD=%CD%
cd %~dp0
echo Generating build version...
cscript.exe //Nologo version.js
cd %MYCD%
