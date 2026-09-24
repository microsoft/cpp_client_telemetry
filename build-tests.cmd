@echo off
cd %~dp0
@setlocal ENABLEEXTENSIONS

set TRANSPORT=%~4
if not defined TRANSPORT set TRANSPORT=WinHTTP
if /I "%TRANSPORT%"=="WinInet" (
  set TRANSPORT_PROPERTY=/p:MATSDK_USE_WININET=true
) else if /I "%TRANSPORT%"=="WinHTTP" (
  set TRANSPORT_PROPERTY=/p:MATSDK_USE_WININET=false
) else (
  echo ERROR: Unknown HTTP transport "%TRANSPORT%". Expected WinHTTP or WinInet.
  exit /b 2
)
echo HTTP transport: %TRANSPORT%

set CUSTOM_PROPS=
if not "%~3"=="" (
  if not exist "%~f3" (
    goto custom_props_missing
  )
  if /I not "%~x3"==".props" (
    if /I not "%~x3"==".targets" (
      goto custom_props_invalid_type
    )
  )
  set CUSTOM_PROPS="/p:ForceImportBeforeCppTargets=%~f3"
  echo Using custom properties file for the build:
  echo %CUSTOM_PROPS%
)

goto after_custom_props_validation

:custom_props_missing
echo ERROR: Custom build input not found: %~3
echo        Pass an existing MSBuild .props or .targets file to ForceImportBeforeCppTargets.
exit /b 1

:custom_props_invalid_type
echo ERROR: Custom build input must be an MSBuild .props or .targets file: %~3
echo        Pass the MSBuild import file, not the CONFIG_CUSTOM_H header.
exit /b 1

:after_custom_props_validation
call tools\gen-version.cmd

if DEFINED GIT_PULL_TOKEN (
  rd /s /q lib\modules
  git clone https://%GIT_PULL_TOKEN%:x-oauth-basic@github.com/microsoft/cpp_client_telemetry_modules.git lib\modules
)

set GTEST_PATH=third_party\googletest
if NOT EXIST %GTEST_PATH%\CMakeLists.txt (
  git clone --depth 1 --branch release-1.12.1 https://github.com/google/googletest %GTEST_PATH%
)

set PLATFORM=

REM Possible platforms: Win32|x64
set PLAT=%1
REM Possible configurations: Release|Debug
set CONFIGURATION=%2

set MAXCPUCOUNT=%NUMBER_OF_PROCESSORS%
set SOLUTION=Solutions\MSTelemetrySDK.sln

msbuild %SOLUTION% /target:sqlite:Rebuild,zlib:Rebuild,Tests\gmock:Rebuild,Tests\gtest:Rebuild,Tests\UnitTests:Rebuild,Tests\FuncTests:Rebuild /p:BuildProjectReferences=true /maxcpucount:%MAXCPUCOUNT% /detailedsummary /p:Configuration=%CONFIGURATION% /p:Platform=%PLAT% %TRANSPORT_PROPERTY% %CUSTOM_PROPS%
if not "%ERRORLEVEL%"=="0" exit /b %ERRORLEVEL%
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File .github\scripts\run-with-timeout.ps1 -FilePath Solutions\out\%CONFIGURATION%\%PLAT%\UnitTests\UnitTests.exe -TimeoutSeconds 600 -Label UnitTests-%CONFIGURATION%-%PLAT%-%TRANSPORT%
if not "%ERRORLEVEL%"=="0" exit /b %ERRORLEVEL%
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File .github\scripts\run-with-timeout.ps1 -FilePath Solutions\out\%CONFIGURATION%\%PLAT%\FuncTests\FuncTests.exe -TimeoutSeconds 600 -Label FuncTests-%CONFIGURATION%-%PLAT%-%TRANSPORT%
if not "%ERRORLEVEL%"=="0" exit /b %ERRORLEVEL%
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File .github\scripts\run-with-timeout.ps1 -FilePath Solutions\out\%CONFIGURATION%\%PLAT%\FuncTests\FuncTests.exe -ProcessArguments "--gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager" -ProcessCount 2 -TimeoutSeconds 600 -Label FuncTests-concurrent-%CONFIGURATION%-%PLAT%-%TRANSPORT%
if not "%ERRORLEVEL%"=="0" exit /b %ERRORLEVEL%
