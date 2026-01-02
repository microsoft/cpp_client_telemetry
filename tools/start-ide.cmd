set "ROOT=%~dp0\.."

REM Specify custom build options: uncomment line below pointing to custom build configuration
REM set "CUSTOM_PROPS_VS=%ROOT%\Solutions\build.compact-min.props"

REM This script assumes that local IDE is Visual Studio 2022 Enterprise + toolchains (vc143, etc.)
REM Edit line below if your IDE is different than Visual Studio 2022
set "IDE=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\devenv.exe"

"%IDE%" "%ROOT%\Solutions\MSTelemetrySDK.sln"
