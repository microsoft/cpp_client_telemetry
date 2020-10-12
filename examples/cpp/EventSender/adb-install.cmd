set ANDROID_HOME=C:/Android/android-sdk
set "PATH=%ANDROID_NDK_HOME%;%ANDROID_HOME%\platform-tools;%PATH%"
adb shell "mkdir /data/bin"
adb push out/EventSender /data/bin
adb shell "cd /data/bin && chmod 0777 ./EventSender && ./EventSender"
