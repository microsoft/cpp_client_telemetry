set "PATH=%~dp0\..\..\..\;%PATH%"
copy /Y %~dp0\..\..\..\..\lib\bond\CsProtocol.bond %~dp0
gbc.exe c# CSProtocol.bond
move *.cs %~dp0\..
