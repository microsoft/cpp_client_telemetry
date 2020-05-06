@echo off
pushd "%~dp0"
set "PATH=%CD%\Python3.7;%PATH%"

REM Install latest node 14.x+
choco install nodejs

if not exist emsdk git clone https://github.com/emscripten-core/emsdk.git

cd emsdk
git pull
call emsdk install latest
call emsdk activate latest

call emsdk install mingw-4.6.2-32bit
call emsdk activate mingw-4.6.2-32bit
