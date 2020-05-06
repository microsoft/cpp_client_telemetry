if not exist build mkdir build
cd build

pushd "%~dp0"
call emsdk\emsdk_env.bat
popd

set CC=emcc
set CXX=em++
call emcmake.bat cmake -G "MinGW Makefiles" ..
call emmake.bat mingw32-make
