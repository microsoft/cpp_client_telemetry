@echo off
set PROJECT_DIR=%~dp0

@mkdir %PROJECT_DIR%\include 2> NUL
robocopy %PROJECT_DIR%..\..\..\lib\include\public\ %PROJECT_DIR%\include

@mkdir %PROJECT_DIR%\lib\%1\%2 2> NUL
robocopy %PROJECT_DIR%..\..\..\Solutions\out\%1\%2\win32-dll\ %PROJECT_DIR%\lib\%1\%2 "*.lib""

@mkdir %PROJECT_DIR%\%1\%2 2> NUL
robocopy %PROJECT_DIR%..\..\..\Solutions\out\%1\%2\win32-dll\ %PROJECT_DIR%\lib\%1\%2
robocopy %PROJECT_DIR%..\..\..\Solutions\out\%1\%2\win32-dll\ %3
exit /b 0
