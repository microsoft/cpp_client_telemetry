@echo off

echo WARNING!!! ****************************************************************************************** !!!WARNING
echo WARNING!!! This part of the build process requires Visual Studio 2017 IDE to be installed on machine. !!!WARNING
echo WARNING!!! llvm.vsix extension (Clang LLVM support for Visual Studio) is INCOMPATIBLE with vs2019.    !!!WARNING
echo WARNING!!! vs2019 build fails with an error: The build tools for LLVM (Platform Toolset = 'LLVM')     !!!WARNING
echo WARNING!!! cannot be found. Since Visual Studio 2019-preview includes built-in support for Clang,     !!!WARNING
echo WARNING!!! feel free to contribute a 'build-cmake-clang-vs2019.cmd' script. The only change should    !!!WARNING
echo WARNING!!! be to change the LLVM_VER variable below to clang_cl_x64 ... Except that there could be    !!!WARNING
echo WARNING!!! another issue (June 2019) here: https://gitlab.kitware.com/cmake/cmake/-/issues/20180      !!!WARNING
echo WARNING!!! The other alternative is to actually install Visual Studio 2017 IDE side-by-side....       !!!WARNING
echo WARNING!!! ****************************************************************************************** !!!WARNING
timeout 5

cd %~dp0
setlocal enableextensions
setlocal enabledelayedexpansion
set ROOT=%~dp0

REM ********************************************************************
REM Use cmake
REM ********************************************************************
set "PATH=C:\Program Files\CMake\bin\;%PATH%"

REM ********************************************************************
REM Use clang compiler
REM ********************************************************************
set CLANG_PATH="C:\Program Files\LLVM\bin"
set CC=%CLANG_PATH%\clang.exe
set CXX=%CLANG_PATH%\clang++.exe
set LLVM_VER=LLVM

REM ********************************************************************
REM Set output directory, clean and/or create as-needed
REM ********************************************************************
set OUTDIR=%ROOT%\Solutions\out
if "%1" == "clean" (
  @rmdir /s /q %OUTDIR%
)
if not exist "%OUTDIR%" mkdir %OUTDIR%

REM ********************************************************************
REM Build all deps using MSVC - Visual Studio 2017 (15)
REM ********************************************************************
if "%1" == "nodeps" goto NODEPS
call tools\build-deps.cmd
:NODEPS

cd %OUTDIR%

REM ********************************************************************
REM Invoke the build script
REM ********************************************************************
set CMAKE_PACKAGE_TYPE=tgz
for %%a in ( m32 m64 ) do (
  for %%c in ( Release ) do (
    if "%%a"=="m32" (
      set ARCH=Win32
      set ARCH_GEN=
    )
    if "%%a"=="m64" (
      set ARCH=x64
      set ARCH_GEN= Win64
    )
    @mkdir %OUTDIR%\%%c\!ARCH!
    cd %OUTDIR%\%%c\!ARCH!
    set "CFLAGS=-%%a"
    set "CXXFLAGS=-%%a -Wc++11-compat-pedantic -Wno-c++98-compat -Wno-everything"
    cmake -G"Visual Studio 15 2017!ARCH_GEN!" ^
      -T"%LLVM_VER%" ^
      -DTARGET_ARCH=!ARCH! ^
      -DBUILD_SHARED_LIBS=OFF ^
      -DCMAKE_BUILD_TYPE=%%c ^
      -DCMAKE_PACKAGE_TYPE=%CMAKE_PACKAGE_TYPE% ^
      -DDEFAULT_PAL_IMPLEMENTATION=WIN32 ^
      %ROOT%
    cmake --build . --config %%c -- /p:Configuration=%%c
  )
)

cd %ROOT%
