@echo off
set PROJECT_DIR=%~dp0

@mkdir %PROJECT_DIR%\include
copy /Y %PROJECT_DIR%..\..\..\lib\include\public\*.* %PROJECT_DIR%\include

@mkdir %PROJECT_DIR%\lib\%1\%2
copy /Y %PROJECT_DIR%..\..\..\Solutions\out\%1\%2\win32-dll\*.lib %PROJECT_DIR%\lib\%1\%2

@mkdir %PROJECT_DIR%\%1\%2
copy /Y %PROJECT_DIR%..\..\..\Solutions\out\%1\%2\win32-dll\*.* %PROJECT_DIR%\lib\%1\%2
copy /Y %PROJECT_DIR%..\..\..\Solutions\out\%1\%2\win32-dll\*.* %3
exit /b 0
