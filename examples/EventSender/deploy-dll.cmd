@echo off
set PROJECT_DIR=%~dp0
copy %PROJECT_DIR%\lib\%1\%2\*.* %3
exit /b 0
