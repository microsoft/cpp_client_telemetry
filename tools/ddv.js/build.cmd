@ECHO off
PUSHD "%~dp0"
CD /d "%~dp0"

REM Adjust this to point to emsdk_env.bat
CALL ..\emsdk\emsdk_env.bat

set "PATH=%EMSDK%;%PATH%"
echo Testing emscripten installation...
set "OUTDIR=%CD%\out"
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

REM -s LINKABLE=1
REM -s MODULARIZE=1
REM -s EXPORT_ES6=1
REM -s SIDE_MODULE=1
emcc -O0 main.cpp -std=c++11 -o main.wasm -s WASM=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s EXPORTED_FUNCTIONS="['_main', '_getHello', '_sayHello']"

echo "[ DONE ]"
