@echo off
set PROJECT_DIR=%~dp0

@mkdir %PROJECT_DIR%\include
copy /Y %PROJECT_DIR%..\..\..\lib\include\public\mat.h %PROJECT_DIR%\include
copy /Y %PROJECT_DIR%..\..\..\lib\include\public\ctmacros.hpp %PROJECT_DIR%\include

@mkdir %PROJECT_DIR%\lib\%1\%2
robocopy %PROJECT_DIR%..\..\..\Solutions\out\%1\%2\win32-dll %3 *.dll /S
exit /b 0
