@echo off
echo WARNING!!! ****************************************************************************************** !!!WARNING
echo WARNING!!! This part of the build process requires Visual Studio 2017 IDE to be installed on machine: !!!WARNING
echo WARNING!!! llvm.vsix extension (Clang LLVM support for Visual Studio) is INCOMPATIBLE with vs2019.    !!!WARNING
echo WARNING!!! vs2019 build fails with an error: The build tools for LLVM (Platform Toolset = 'LLVM')     !!!WARNING
echo WARNING!!! cannot be found. If you intend to use vs2019, build with build-cmake-clang-vs2019.cmd      !!!WARNING
echo WARNING!!! ****************************************************************************************** !!!WARNING
timeout 5

set VSTOOLS_VERSION=vs2017
cd %~dp0

echo Update all public submodules...
git -c submodule."lib/modules".update=none submodule update --init --recursive

if DEFINED GIT_PULL_TOKEN (
  rd /s /q lib\modules
  git clone https://%GIT_PULL_TOKEN%:x-oauth-basic@github.com/microsoft/cpp_client_telemetry_modules.git lib\modules
)

call tools\vcvars.cmd

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
