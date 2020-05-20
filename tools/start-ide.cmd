set "ROOT=%~dp0\.."
REM Specify custom build options
set "CUSTOM_PROPS_VS=%ROOT%\Solutions\build.compact-min.props"
set "IDE=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\Common7\IDE\devenv.exe"
"%IDE%" "%ROOT%\Solutions\MSTelemetrySDK.sln"
