@echo off
set PROJECT_DIR=%~dp0

@mkdir %PROJECT_DIR%\include
copy %PROJECT_DIR%..\..\lib\include\public\mat.h %PROJECT_DIR%\include
copy %PROJECT_DIR%..\..\lib\include\public\Version.h %PROJECT_DIR%\include
copy %PROJECT_DIR%..\..\lib\include\public\ctmacros.hpp %PROJECT_DIR%\include

@mkdir %PROJECT_DIR%\lib\%1\%2
copy %PROJECT_DIR%..\..\Solutions\out\%1\%2\win32-dll\*.lib %PROJECT_DIR%\lib\%1\%2
copy %PROJECT_DIR%..\..\Solutions\out\Release\x64\lib\Release\*.lib %PROJECT_DIR%\lib\%1\%2

exit /b 0