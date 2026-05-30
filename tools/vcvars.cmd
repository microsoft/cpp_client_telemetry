@echo off
REM
REM Make sure to enable the 'Visual C++ ATL' components for all platforms during the setup.
REM
REM This build script auto-detects and configures Visual Studio in the following order:
REM 1. Visual Studio 2017 Enterprise
REM 2. Visual Studio 2017 BuildTools
REM 3. Visual Studio 2019 Enterprise
REM 4. Visual Studio 2019 Community
REM 5. Visual Studio 2019 BuildTools
REM 6. Visual Studio 2022 Enterprise
REM 7. Visual Studio 2022 Professional
REM 8. Visual Studio 2022 Community
REM 9. Visual Studio 2022 BuildTools
REM 10. Visual Studio 2026 Enterprise
REM 11. Visual Studio 2026 Professional
REM 12. Visual Studio 2026 Community
REM 13. Visual Studio 2026 BuildTools
REM

REM 1st parameter - Visual Studio version
if "%1" neq "" (
  goto %1
)

if "%VSTOOLS_VERSION%" neq "" (
  goto %VSTOOLS_VERSION%
)

REM vs2017 Enterprise
:vs2017
:vs2017_enterprise
set "VSDEVCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"
set VSVERSION=2017
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise"
  echo Building with vs2017 Enterprise...
  call "%VSDEVCMD%"
  goto tools_configured
)

REM vs2017 BuildTools
:vs2017_buildtools
set "VSDEVCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\BuildTools"
  echo Building with vs2017 BuildTools...
  call "%VSDEVCMD%"
  goto tools_configured
)

REM vs2019 Enterprise
:vs2019
:vs2019_enterprise
set VSVERSION=2019
set "VSDEVCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise"
  echo Building with vs2019 Enterprise...
  call "%VSDEVCMD%"
  goto tools_configured
)

REM vs2019 Community
:vs2019_community
set "VSDEVCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community"
  echo Building with vs2019 Community...
  call "%VSDEVCMD%"
  goto tools_configured
)

REM vs2019 BuildTools
:vs2019_buildtools
set "VSDEVCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools"
  echo Building with vs2019 BuildTools...
  call "%VSDEVCMD%"
  goto tools_configured
)

:vs2022
:vs2022_enterprise
SET VSVERSION=2022
set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise"
  echo Building with vs2022 Enterprise...
  call "%VSDEVCMD%"
  goto tools_configured
)

:vs2022_professional
SET VSVERSION=2022
set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2022\Professional"
  echo Building with vs2022 Professional...
  call "%VSDEVCMD%"
  goto tools_configured
)

:vs2022_community
SET VSVERSION=2022
set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2022\Community"
  echo Building with vs2022 Community...
  call "%VSDEVCMD%"
  goto tools_configured
)

:vs2022_buildtools
SET VSVERSION=2022
set "VSDEVCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools"
  echo Building with vs2022 BuildTools...
  call "%VSDEVCMD%"
  goto tools_configured
)

:vs2026
:vs2026_enterprise
SET VSVERSION=2026
set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\2026\Enterprise\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2026\Enterprise"
  echo Building with vs2026 Enterprise...
  call "%VSDEVCMD%"
  goto tools_configured
)

set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\18\Enterprise"
  echo Building with vs2026 Enterprise...
  call "%VSDEVCMD%"
  goto tools_configured
)

:vs2026_professional
SET VSVERSION=2026
set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\2026\Professional\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2026\Professional"
  echo Building with vs2026 Professional...
  call "%VSDEVCMD%"
  goto tools_configured
)

set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\18\Professional\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\18\Professional"
  echo Building with vs2026 Professional...
  call "%VSDEVCMD%"
  goto tools_configured
)

:vs2026_community
SET VSVERSION=2026
set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\2026\Community\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2026\Community"
  echo Building with vs2026 Community...
  call "%VSDEVCMD%"
  goto tools_configured
)

set "VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\18\Community"
  echo Building with vs2026 Community...
  call "%VSDEVCMD%"
  goto tools_configured
)

:vs2026_buildtools
SET VSVERSION=2026
set "VSDEVCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\2026\BuildTools\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2026\BuildTools"
  echo Building with vs2026 BuildTools...
  call "%VSDEVCMD%"
  goto tools_configured
)

set "VSDEVCMD=%ProgramFiles(x86)%\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat"
if exist "%VSDEVCMD%" (
  set "VSINSTALLDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\18\BuildTools"
  echo Building with vs2026 BuildTools...
  call "%VSDEVCMD%"
  goto tools_configured
)

echo WARNING:*********************************************
echo WARNING: cannot auto-detect Visual Studio version !!!
echo WARNING:*********************************************
set VSTOOLS_NOTFOUND=1
set VSVERSION=
exit /b 0

:tools_configured
