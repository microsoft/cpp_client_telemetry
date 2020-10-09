set ANDROID_BUILD_TOOLS=C:\Android\android-sdk\build-tools\30.0.2
set "PATH=%ANDROID_BUILD_TOOLS%;C:\Program Files\LLVM\bin;%PATH%"
set OUTDIR=%CD%
pushd com\microsoft\telemetry
REM aidl.exe --lang=cpp -o %OUTDIR%\gen2 -h %OUTDIR%\gen2 --transaction_names -t -a ITelemetryAgent.aidl
aidl.exe --lang=cpp -o %OUTDIR%\gen2 -h %OUTDIR%\gen2 ITelemetryAgent.aidl
pushd %OUTDIR%\gen2\com\microsoft\telemetry
clang-format -i *.h *.cpp
popd
popd
