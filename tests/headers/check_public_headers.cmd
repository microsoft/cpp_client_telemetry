@echo off
REM Copyright (c) Microsoft Corporation. All rights reserved.
REM SPDX-License-Identifier: Apache-2.0
REM
REM Public header gate (MSVC). Compiles each public SDK header on its own under
REM /W4 /WX, mirroring how ONNX Runtime / Foundry Local compile their own C++
REM translation units on Windows. STL headers are treated as external
REM (/external:W0) so only the SDK's headers are gated. Exits non-zero if any
REM header fails to compile or emits a warning.
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%..\.."
set "PUB=%REPO_ROOT%\lib\include\public"

REM Enter the MSVC x64 developer environment via vswhere (portable across runners).
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo error: vswhere.exe not found 1>&2
  exit /b 2
)
set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH (
  echo error: no Visual Studio installation found 1>&2
  exit /b 2
)
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 (
  echo error: failed to initialize the MSVC environment 1>&2
  exit /b 2
)

set "WORK=%TEMP%\pubhdrgate"
if exist "%WORK%" rmdir /s /q "%WORK%"
mkdir "%WORK%"

REM /W4 /WX matches ORT; /external:W0 suppresses STL warnings so only our headers gate.
set "FLAGS=/nologo /std:c++17 /permissive- /W4 /WX /EHsc /experimental:external /external:anglebrackets /external:W0"
set "FAIL=0"
set "OKC=0"

echo == cl (c++17, /W4 /WX) ==
for %%h in ("%PUB%\*.hpp") do (
  set "NAME=%%~nxh"
  REM Skip implementation-fragment headers not meant to be included standalone
  REM (VariantType.hpp is included by Variant.hpp, which defines VariantMap/VariantArray first).
  if /I not "!NAME!"=="VariantType.hpp" (
    > "%WORK%\tu.cpp" echo #include "!NAME!"
    >> "%WORK%\tu.cpp" echo int main^(^){return 0;}
    cl %FLAGS% /I "%PUB%" /Zs "%WORK%\tu.cpp" > "%WORK%\err.txt" 2>&1
    if errorlevel 1 (
      echo   FAIL: !NAME!
      type "%WORK%\err.txt"
      set "FAIL=1"
    ) else (
      set /a OKC+=1
    )
  )
)

if "%FAIL%"=="1" (
  echo Public header gate FAILED.
  exit /b 1
)
echo Public header gate passed. ^(!OKC! headers^)
exit /b 0
