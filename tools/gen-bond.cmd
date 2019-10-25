@echo off

cd %~dp0
set PATH=C:\Python27\bin;%CD%;%PATH%
set TOOLS=%CD%

echo building JSON schema from CsProtocol.bond ...
cd ..\lib\bond\generated
gbc schema ..\CsProtocol.bond

echo generating readers/writers...
python.exe %TOOLS%\bondjson2cpp.py CsProtocol.json

echo [ DONE ]
