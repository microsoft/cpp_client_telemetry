@REM This script allows to download a file to local machine. First argument is URL
set "PATH=C:\Windows;C:\Windows\System32;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\Program Files\Git\bin"
@powershell -File download.ps1 %1
