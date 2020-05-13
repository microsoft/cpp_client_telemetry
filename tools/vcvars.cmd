REM vs2017 BuildTools
set TOOLS_VS2017="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"
if exist %TOOLS_VS2017% (
  echo Building with vs2017 BuildTools...
  call %TOOLS_VS2017%
  goto tools_configured
)

REM vs2017 Enterprise
set TOOLS_VS2017_ENTERPRISE="C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"
if exist %TOOLS_VS2017_ENTERPRISE% (
  echo Building with vs2017 Enterprise...
  call %TOOLS_VS2017_ENTERPRISE%
  goto tools_configured
)

REM vs2019 BuildTools
set TOOLS_VS2019="C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"
if exist %TOOLS_VS2019% (
  echo Building with vs2019 BuildTools...
  call %TOOLS_VS2017%
  goto tools_configured
)

REM vs2019 Community
set TOOLS_VS2019_COMMUNITY="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
if exist %TOOLS_VS2019_COMMUNITY% (
  echo Building with vs2019 Community...
  call %TOOLS_VS2019_COMMUNITY%
  goto tools_configured
)

REM vs2019 Enterprise
set TOOLS_VS2019_ENTERPRISE="C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat"
if exist %TOOLS_VS2019_ENTERPRISE% (
  echo Building with vs2019 Enterprise...
  call %TOOLS_VS2019_ENTERPRISE%
  goto tools_configured
)

echo WARNING:*********************************************
echo WARNING: cannot auto-detect Visual Studio version !!!
echo WARNING:*********************************************

:tools_configured
