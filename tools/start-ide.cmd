set "ROOT=%~dp0\.."

REM Specify custom build options: uncomment line below pointing to custom build configuration
REM set "CUSTOM_PROPS_VS=%ROOT%\Solutions\build.compact-min.props"

REM This script assumes that local IDE is Visual Studio 2019 Enterprise + toolchains (vc141, vc142, etc.)
REM Edit line below if your IDE is different than Visual Studio 2019
set "IDE=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\Common7\IDE\devenv.exe"

"%IDE%" "%ROOT%\Solutions\MSTelemetrySDK.sln"
