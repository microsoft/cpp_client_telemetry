@echo off
echo Creating SKU %1 ...

set DST_NAME=%1
set SRC_NAME=%2
set MSC_VER=%3

set REL_SRC=%SRCDIR%\Release
set DBG_SRC=%SRCDIR%\Debug

if "%3" == "" GOTO proceed
set REL_SRC=%REL_SRC%.%3
set DBG_SRC=%DBG_SRC%.%3

:proceed

echo Release = %REL_SRC%
echo Debug   = %DBG_SRC%

if exist "%REL_SRC%\Win32\%SRC_NAME%" (
  robocopy %REL_SRC%\Win32\%SRC_NAME%   "%OUTDIR%\lib\%DST_NAME%\x86\Release" *.pri *.winmd *.dll *.pdb *.lib *.map *.exp
  robocopy %DBG_SRC%\Win32\%SRC_NAME%   "%OUTDIR%\lib\%DST_NAME%\x86\Debug"   *.pri *.winmd *.dll *.pdb *.lib *.map *.exp
)

if exist "%REL_SRC%\x64\%SRC_NAME%" (
  robocopy %REL_SRC%\x64\%SRC_NAME%     "%OUTDIR%\lib\%DST_NAME%\x64\Release" *.pri *.winmd *.dll *.pdb *.lib *.map *.exp
  robocopy %DBG_SRC%\x64\%SRC_NAME%     "%OUTDIR%\lib\%DST_NAME%\x64\Debug"   *.pri *.winmd *.dll *.pdb *.lib *.map *.exp
)

if exist "%REL_SRC%\ARM\%SRC_NAME%" (
  robocopy %REL_SRC%\ARM\%SRC_NAME%     "%OUTDIR%\lib\%DST_NAME%\ARM\Release" *.pri *.winmd *.dll *.pdb *.lib *.map *.exp
  robocopy %DBG_SRC%\ARM\%SRC_NAME%     "%OUTDIR%\lib\%DST_NAME%\ARM\Debug"   *.pri *.winmd *.dll *.pdb *.lib *.map *.exp
)

if exist "%REL_SRC%\ARM64\%SRC_NAME%" (
  robocopy %REL_SRC%\ARM64\%SRC_NAME%     "%OUTDIR%\lib\%DST_NAME%\ARM64\Release" *.pri *.winmd *.dll *.pdb *.lib *.map *.exp
  robocopy %DBG_SRC%\ARM64\%SRC_NAME%     "%OUTDIR%\lib\%DST_NAME%\ARM64\Debug"   *.pri *.winmd *.dll *.pdb *.lib *.map *.exp
)
