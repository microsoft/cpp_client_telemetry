@echo off
setlocal enabledelayedexpansion
if %1==format (  
  where clang-format > NUL
  if %ERRORLEVEL% neq 0 (
    echo clang-format.exe not found in PATH!
    echo Assuming default path for LLVM tools...
    set PATH="C:\Program Files\LLVM\bin;!PATH!"
  )
  set "TMPFILE=%2-~%RANDOM%"
  clang-format %2 > !TMPFILE!
  type !TMPFILE! > %2
  del !TMPFILE!
)
endlocal
