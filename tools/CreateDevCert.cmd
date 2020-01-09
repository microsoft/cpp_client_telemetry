@echo off
set "PATH=C:\Windows\System32\WindowsPowerShell\v1.0\;%PATH%"
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "& '.\CreateDevCert.ps1' %*"
copy /Y TestApp.pfx ..\examples\cpp\SampleCppUWP\TestApp.pfx
