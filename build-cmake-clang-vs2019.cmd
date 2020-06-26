@echo off

set VSTOOLS_VERSION=vs2019
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
set LLVM_VER=ClangCL

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
      set ARCH_GEN= Win32
    )
    if "%%a"=="m64" (
      set ARCH=x64
      set ARCH_GEN= x64
    )
    @mkdir %OUTDIR%\%%c\!ARCH!
    cd %OUTDIR%\%%c\!ARCH!
    set "CFLAGS=-%%a"
    set "CXXFLAGS=-%%a -Wc++11-compat-pedantic -Wno-c++98-compat -Wno-everything"
    cmake -G"Visual Studio 16 2019" -A !ARCH_GEN! ^
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
