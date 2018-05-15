@echo off

rmdir /S /Q files
mkdir files

set DLLNAME=ClientTelemetry
set PROJNAME=win32-dll

echo Copy vs2015 flavor...
xcopy /Y /I /F %CD%\..\..\Solutions\out\Release\Win32\%PROJNAME%\%DLLNAME%.dll		files\lib\%PROJNAME%-vs2015\Release\x86\
xcopy /Y /I /F %CD%\..\..\Solutions\out\Release\x64\%PROJNAME%\%DLLNAME%.dll		files\lib\%PROJNAME%-vs2015\Release\x64\
xcopy /Y /I /F %CD%\..\..\Solutions\out\Debug\Win32\%PROJNAME%\%DLLNAME%.dll		files\lib\%PROJNAME%-vs2015\Debug\x86\
xcopy /Y /I /F %CD%\..\..\Solutions\out\Debug\x64\%PROJNAME%\%DLLNAME%.dll		files\lib\%PROJNAME%-vs2015\Debug\x64\

echo Copy vs2013 flavor...
xcopy /Y /I /F %CD%\..\..\Solutions\out\Release.vs2013\Win32\%PROJNAME%\%DLLNAME%.dll	files\lib\%PROJNAME%-vs2013\Release\x86\
xcopy /Y /I /F %CD%\..\..\Solutions\out\Release.vs2013\x64\%PROJNAME%\%DLLNAME%.dll	files\lib\%PROJNAME%-vs2013\Release\x64\
xcopy /Y /I /F %CD%\..\..\Solutions\out\Debug.vs2013\Win32\%PROJNAME%\%DLLNAME%.dll	files\lib\%PROJNAME%-vs2013\Debug\x86\
xcopy /Y /I /F %CD%\..\..\Solutions\out\Debug.vs2013\x64\%PROJNAME%\%DLLNAME%.dll	files\lib\%PROJNAME%-vs2013\Debug\x64\

echo Run codesigner...
.\bin\x86\Release\codesign.exe %CD%\xml\win32-dll.xml -dir %CD%
