@echo off
REM Copyright (c) Microsoft Corporation. All rights reserved.
REM SPDX-License-Identifier: Apache-2.0
REM
REM Public header gate (MSVC). Compiles each public SDK header on its own under
REM /W4 /WX, mirroring how ONNX Runtime / Foundry Local compile their own C++
REM translation units on Windows. STL/Windows SDK headers are treated as external
REM (/external:W0) so only the SDK's headers are gated. Exits non-zero if any
REM header fails to compile or emits a warning. Also compiles mat.h as C11 (/TC).
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%..\.."
set "PUB=%REPO_ROOT%\lib\include\public"
set "C_API_HEADER=mat.h"

REM Fail fast if the public header directory is missing or miscomputed, otherwise
REM the header loop below would run zero times and the gate would silently "pass"
REM without compiling anything -- a false negative.
if not exist "%PUB%" (
  echo error: public header directory not found: %PUB% 1>&2
  exit /b 2
)

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

REM Unique work directory under the repository so concurrent invocations on the
REM same machine do not clobber each other's temporary translation units.
set "WORK=%REPO_ROOT%\.public-header-gate_%RANDOM%_%RANDOM%"
if exist "%WORK%" rmdir /s /q "%WORK%"
mkdir "%WORK%"
if errorlevel 1 (
  echo error: failed to create work directory %WORK% 1>&2
  exit /b 2
)

REM /W4 /WX matches ORT; /external:W0 suppresses platform/STL warnings so only our headers gate.
set "CXX_COMMON=/nologo /permissive- /W4 /WX /EHsc /experimental:external /external:anglebrackets /external:W0"
set "C_COMMON=/nologo /std:c11 /TC /W4 /WX /experimental:external /external:anglebrackets /external:W0"
set "FAIL=0"
set "TOTAL=0"

REM MSVC does not expose a /std:c++11 switch; /std:c++14 is its lowest selectable mode.
call :RunCxxHeaders c++14 /std:c++14 "cl (c++14, /W4 /WX)"
call :RunCxxHeaders c++17 /std:c++17 "cl (c++17, /W4 /WX)"
call :RunCHeader

rmdir /s /q "%WORK%" 2>nul

if "%FAIL%"=="1" (
  echo Public header gate FAILED.
  exit /b 1
)
echo Public header gate passed. ^(!TOTAL! checks^)
exit /b 0

:RunCxxHeaders
set "STD_NAME=%~1"
set "STD_FLAG=%~2"
set "LABEL=%~3"
set "OKC=0"
set "FAILC=0"
echo == %LABEL% ==
for %%h in ("%PUB%\*.hpp" "%PUB%\*.h") do (
  set "NAME=%%~nxh"
  REM Skip implementation-fragment headers not meant to be included standalone
  REM (VariantType.hpp is included by Variant.hpp, which defines VariantMap/VariantArray first).
  if /I not "!NAME!"=="VariantType.hpp" (
    > "%WORK%\tu_!STD_NAME!.cpp" echo #include "!NAME!"
    >> "%WORK%\tu_!STD_NAME!.cpp" echo int main^(^){return 0;}
    cl %CXX_COMMON% %STD_FLAG% /I "%PUB%" /I "%REPO_ROOT%\lib\include" /Zs "%WORK%\tu_!STD_NAME!.cpp" > "%WORK%\err.txt" 2>&1
    if errorlevel 1 (
      echo   FAIL: !NAME!
      type "%WORK%\err.txt"
      set "FAIL=1"
      set /a FAILC+=1
    ) else (
      set /a OKC+=1
      set /a TOTAL+=1
    )
  )
)
REM No headers compiled means PUB matched nothing -- treat it as a failure rather
REM than a silent pass.
if "!OKC!"=="0" if "!FAILC!"=="0" (
  echo error: no public headers found under %PUB% 1>&2
  set "FAIL=1"
)
echo   %LABEL%: !OKC! passed, !FAILC! failed
exit /b 0

:RunCHeader
set "LABEL=cl (C11 mat.h, /TC, /W4 /WX)"
echo == !LABEL! ==
if not exist "%PUB%\%C_API_HEADER%" (
  echo error: C API header not found: %PUB%\%C_API_HEADER% 1>&2
  set "FAIL=1"
  echo   !LABEL!: 0 passed, 1 failed
  exit /b 0
)
> "%WORK%\tu_mat_c11.c" echo #include "%C_API_HEADER%"
>> "%WORK%\tu_mat_c11.c" echo int main^(void^){return 0;}
cl %C_COMMON% /I "%PUB%" /I "%REPO_ROOT%\lib\include" /Zs "%WORK%\tu_mat_c11.c" > "%WORK%\err.txt" 2>&1
if errorlevel 1 (
  echo   FAIL: %C_API_HEADER%
  type "%WORK%\err.txt"
  set "FAIL=1"
  echo   !LABEL!: 0 passed, 1 failed
) else (
  set /a TOTAL+=1
  echo   !LABEL!: 1 passed, 0 failed
)
exit /b 0
