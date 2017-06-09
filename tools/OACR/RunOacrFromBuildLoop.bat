@echo off

if /i '%BUILD_REPOSITORY_LOCALPATH%' == '' (
echo This script is only intended to be run by the build agent!
goto :EOF
)

SETLOCAL ENABLEEXTENSIONS

set TempBuildPath=%TEMP%\BuildLoop\%BUILD_BUILDNUMBER%
set ARIA_Root=%BUILD_REPOSITORY_LOCALPATH%
set ARIA_OutputRoot=%ARIA_Root%\Solutions\Out
set OACR_OutputFile=%ARIA_OutputRoot%\OACR\Aria.Warnings.xml
set OACR_Root=F:\OACR
set OACR_Version=3.7.25220
set OACR_Flavor=x86
set OACR_Path=%TempBuildPath%\OACR\%OACR_Flavor%
set OACR_OutputRoot=%OACR_Root%\Output
set OACR_Binaries=%OACR_Root%\%OACR_Version%\%OACR_Flavor%

set

if exist %TempBuildPath% rd /s /q %TempBuildPath%
robocopy /e /nfl /ndl /np /A-:R %OACR_Binaries% %OACR_Path%
copy %BUILD_REPOSITORY_LOCALPATH%\tools\OACR\oacr.ini %OACR_Path%
call %ARIA_Root%\RunOACR.bat

if not exist %OACR_OutputRoot% md %OACR_OutputRoot%
if exist %OACR_OutputFile% copy %OACR_OutputFile% %OACR_OutputRoot%\%BUILD_BUILDNUMBER%.OacrWarnings.xml

if exist %TempBuildPath%   rd /s /q %TempBuildPath%
if exist %ARIA_OutputRoot% rd /s /q %ARIA_OutputRoot%
