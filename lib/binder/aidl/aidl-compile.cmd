set ANDROID_BUILD_TOOLS=C:\Android\android-sdk\build-tools\30.0.2
set "PATH=%ANDROID_BUILD_TOOLS%;C:\Program Files\LLVM\bin;%PATH%"
set OUTDIR=%CD%
aidl.exe --lang=cpp -o %OUTDIR%\gen -h %OUTDIR%\gen\include com\microsoft\telemetry\ITelemetryAgent.aidl
mkdir %OUTDIR%\gen\com\microsoft\telemetry
pushd %OUTDIR%\gen\com\microsoft\telemetry
clang-format -i *.h *.cpp
popd
