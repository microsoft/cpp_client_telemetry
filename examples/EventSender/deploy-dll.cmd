@echo off
echo Copy .dll file to target dir...
set PROJECT_DIR=%~dp0
copy %PROJECT_DIR%\lib\%2\%1\*.* %3
exit /b 0
