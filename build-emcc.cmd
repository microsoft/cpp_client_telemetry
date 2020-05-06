REM ********************************************************************
REM Use cmake
REM ********************************************************************
set "PATH=C:\Program Files\CMake\bin\;%PATH%"

set "WORKSPACE=%CD%"
set "PATH=%WORKSPACE%\tools;%PATH%"
set "SYSROOT=%WORKSPACE%\out\emscripten"
set SYSROOT=%SYSROOT:\=/%

REM Build zlib for Emscripten
if not exist %SYSROOT% mkdir %SYSROOT%
cd "%SYSROOT%"
git clone https://github.com/emscripten-ports/zlib
cd zlib
call build-emcc.cmd

REM Build decoder module
cd "%WORKSPACE%\tools\ddv.js\decoder"
rmdir build /s /q
call build-emcc.cmd
