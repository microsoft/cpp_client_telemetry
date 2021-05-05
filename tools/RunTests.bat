@echo off
@setlocal ENABLEEXTENSIONS
set TARGETPLATFORM=%1
set TARGETCONFIGURATION=%2

if errorLevel 1 goto end
Solutions\out\%TARGETCONFIGURATION%\%TARGETPLATFORM%\UnitTests\UnitTests.exe
if errorLevel 1 goto end
Solutions\out\%TARGETCONFIGURATION%\%TARGETPLATFORM%\FuncTests\FuncTests.exe
:end