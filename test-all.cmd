@echo off
set OUTDIR=%CD%\Solutions\out\
cd %OUTDIR%

echo Running x86 unit and functional tests...
.\Release\Win32\UnitTests\UnitTests.exe
.\Release\Win32\FuncTests\FuncTests.exe

echo Running x64 unit and functional tests...
.\Release\x64\UnitTests\UnitTests.exe
.\Release\x64\FuncTests\FuncTests.exe
