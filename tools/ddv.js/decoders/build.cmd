@ECHO off
PUSHD "%~dp0"
CD /d "%~dp0"

REM Adjust this to point to emsdk_env.bat
CALL ..\..\emsdk\emsdk_env.bat

set "PATH=%EMSDK%;%PATH%"
echo Testing emscripten installation...
set "OUTDIR=%CD%\out"
if not exist "%OUTDIR%" mkdir "%OUTDIR%"
REM emcc -O1 hello.c -s MODULARIZE=1 -s EXPORT_ES6=1 -s WASM=1 -s SIDE_MODULE=1 -o "%OUTDIR%\hello.js"
REM emcc -O1 hello.c -s MODULARIZE=1 -s EXPORT_ES6=1 -s WASM=1 -o "%OUTDIR%\hello.js"
REM emcc -O1 hello.c -s WASM=1 -o "%OUTDIR%\hello.js"

emcc main.cpp -std=c++11 -o main.out.js -s WASM=1

echo "[ DONE ]"
