setlocal
set "PATH=%~dp0\..\;%PATH%"
pushd "%~dp0"
copy /Y %~dp0\..\..\lib\bond\CsProtocol.bond %~dp0
gbc.exe c# CSProtocol.bond
del CSProtocol.bond
popd
