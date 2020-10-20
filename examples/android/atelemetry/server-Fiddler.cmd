set ANDROID_HOME=C:/Android/android-sdk
set "PATH=%ANDROID_NDK_HOME%;%ANDROID_HOME%\platform-tools;%PATH%"
REM Start server with proxy
adb shell "cd /data/bin && export http_proxy=10.0.2.2:8888 && echo Using proxy $http_proxy && LD_LIBRARY_PATH=/data/bin:$LD_LIBRARY_PATH ./atelemetry"
