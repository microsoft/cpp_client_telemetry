where /r .. gbc.exe > %TEMP%\gbc.path
set /P GBC=<%TEMP%\gbc.path
%GBC% c# CSProtocol.bond
move *.cs ..

