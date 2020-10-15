set ANDROID_HOME=C:/Android/android-sdk
set "PATH=%ANDROID_NDK_HOME%;%ANDROID_HOME%\platform-tools;%PATH%"
adb root
adb shell killall atelemetry
adb shell mkdir /data/bin
adb push libs/x86_64/atelemetry /data/bin/atelemetry

REM Make sure the libmat.so is built before deploying it!!!
adb push ..\..\..\out\shared\lib\libmat.so /data/bin/libmat.so

REM adb push libs/x86_64/libc++_shared.so /data/bin/libc++_shared.so
adb shell chmod 0777 /data/bin/atelemetry
