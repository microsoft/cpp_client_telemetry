@echo off
pushd "%~dp0"
set "PATH=%CD%\Python3.7;%PATH%"
if not exist emsdk git clone https://github.com/emscripten-core/emsdk.git

cd emsdk
git pull
call emsdk install latest
call emsdk activate latest
