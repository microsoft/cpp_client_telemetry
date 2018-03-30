echo Creating SKU %1 ...

set DST_NAME=%1
set SRC_NAME=%2
set MSC_VER=%3

set REL_SRC=Release
set DBG_SRC=Debug

if "%3" == "" GOTO proceed
set REL_SRC=%REL_SRC%.%3
set DBG_SRC=%DBG_SRC%.%3

:proceed

echo Release = %REL_SRC%
echo Debug   = %DBG_SRC%

cd out

if exist "%REL_SRC%\Win32\%SRC_NAME%\bin" (
  mkdir "%NativeSDKFolder%\lib\%DST_NAME%\x86\Release"
  xcopy %REL_SRC%\Win32\%SRC_NAME%\bin\*.*   "%NativeSDKFolder%\lib\%DST_NAME%\x86\Release" /F /Y /d
  mkdir "%NativeSDKFolder%\lib\%DST_NAME%\x86\Debug"
  xcopy %DBG_SRC%\Win32\%SRC_NAME%\bin\*.*   "%NativeSDKFolder%\lib\%DST_NAME%\x86\Debug" /F /Y /d
)

if exist "%REL_SRC%\x64\%SRC_NAME%\bin" (
  mkdir "%NativeSDKFolder%\lib\%DST_NAME%\x64\Release"
  xcopy %REL_SRC%\x64\%SRC_NAME%\bin\*.*     "%NativeSDKFolder%\lib\%DST_NAME%\x64\Release" /F /Y /d
  mkdir "%NativeSDKFolder%\lib\%DST_NAME%\x64\Debug"
  xcopy %DBG_SRC%\x64\%SRC_NAME%\bin\*.*     "%NativeSDKFolder%\lib\%DST_NAME%\x64\Debug" /F /Y /d
)

if exist "%REL_SRC%\ARM\%SRC_NAME%\bin" (
  mkdir "%NativeSDKFolder%\lib\%DST_NAME%\ARM\Release"
  xcopy %REL_SRC%\ARM\%SRC_NAME%\bin\*.*     "%NativeSDKFolder%\lib\%DST_NAME%\ARM\Release" /F /Y /d
  mkdir "%NativeSDKFolder%\lib\%DST_NAME%\ARM\Debug"
  xcopy %DBG_SRC%\ARM\%SRC_NAME%\bin\*.*     "%NativeSDKFolder%\lib\%DST_NAME%\ARM\Debug" /F /Y /d
)

cd ..
