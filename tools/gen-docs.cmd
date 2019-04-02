set "PATH=%CD%;%PATH%"
set ROOT=%~dp0\..
cd %ROOT%\docs
doxygen.exe -w html headerFile footerFile styleSheetFile cppDoxygenConfig
doxygen.exe cppDoxygenConfig

