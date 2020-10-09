set ANDROID_HOME=C:/Android/android-sdk
set "PATH=%ANDROID_NDK_HOME%;%ANDROID_HOME%\platform-tools;%PATH%"
adb shell "cd /data/bin && LD_LIBRARY_PATH=/data/bin:$LD_LIBRARY_PATH ./atelemetry"
