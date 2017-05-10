@echo off
set OUTDIR=%CD%\..\..\out
cd %OUTDIR%

echo vs2015 x86 Debug
xcopy /Y /D Debug\Win32\win32-dll\bin\*.dll		Debug\Win32\win32-cpp\bin\

echo vs2015 x64 Debug
xcopy /Y /D Debug\x64\win32-dll\bin\*.dll			Debug\x64\win32-cpp\bin\

echo vs2013 x86 Debug
xcopy /Y /D Debug.vs2013\Win32\win32-dll\bin\*.dll	Debug.vs2013\Win32\win32-cpp\bin\

echo vs2013 x64 Debug
xcopy /Y /D Debug.vs2013\x64\win32-dll\bin\*.dll		Debug.vs2013\x64\win32-cpp\bin\

echo vs2015 x32 Release
xcopy /Y /D Release\Win32\win32-dll\bin\*.dll		Release\Win32\win32-cpp\bin\

echo vs2015 x64 Release
xcopy /Y /D Release\x64\win32-dll\bin\*.dll		Release\x64\win32-cpp\bin\

echo vs2013 x32 Release
xcopy /Y /D Release.vs2013\Win32\win32-dll\bin\*.dll	Release.vs2013\Win32\win32-cpp\bin\

echo vs2013 x64 Release
xcopy /Y /D Release.vs2013\x64\win32-dll\bin\*.dll	Release.vs2013\x64\win32-cpp\bin\

exit /b 0