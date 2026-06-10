@echo off
@setlocal ENABLEEXTENSIONS

set TARGETPLATFORM=%1
set CONFIGURATION=%2
set TARGETS=%~3
call :NormalizeTargets "%TARGETS%"

set CUSTOM_PROPS=
if "%~4" == "" goto endCustomProps
set CUSTOM_PROPS=%4
echo Using custom properties file for the build:
echo %CUSTOM_PROPS%
:endCustomProps

call tools\vcvars.cmd
if defined VSTOOLS_NOTFOUND (
  echo.
  echo ERROR: Visual Studio was not detected, so the build cannot continue.
  echo        Install Visual Studio 2019, 2022, or 2026 with the Desktop development with C++ workload,
  echo        or run tools\setup-buildtools.cmd to install the required build tools and components.
  echo        To target a specific installed version, set VSTOOLS_VERSION first, for example:
  echo            set VSTOOLS_VERSION=vs2022
  echo.
  exit /b 1
)

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set platform=
set SOLUTION=Solutions\MSTelemetrySDK.sln

msbuild %SOLUTION% /target:%TARGETS% /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=%CONFIGURATION% /p:Platform=%TARGETPLATFORM% %CUSTOM_PROPS%
exit /b %ERRORLEVEL%

:NormalizeTargets
setlocal ENABLEDELAYEDEXPANSION
set "TARGETS_IN=%~1"
set "NORMALIZED_TARGETS="
for %%T in ("!TARGETS_IN:,=" "!") do (
  set "TARGET=%%~T"
  if /I "!TARGET:~-6!"==":Build" set "TARGET=!TARGET:~0,-6!"
  if defined NORMALIZED_TARGETS (
    set "NORMALIZED_TARGETS=!NORMALIZED_TARGETS!,!TARGET!"
  ) else (
    set "NORMALIZED_TARGETS=!TARGET!"
  )
)
endlocal & set "TARGETS=%NORMALIZED_TARGETS%"
exit /b 0