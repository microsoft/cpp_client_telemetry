set ANDROID_HOME=C:/Android/android-sdk
set ANDROID_NDK_HOME=C:/Android/android-ndk-r21d
set ANDROID_BUILD_TOP=A:/aosp
set ANDROID_SYSROOT_ABI=x86_64
set ANDROID_PRODUCT_NAME=generic_x86_64
set ANDROID_SYSTEM_LIBS=%ANDROID_BUILD_TOP%/out/target/product/%ANDROID_PRODUCT_NAME%/system/lib64
set "PATH=%ANDROID_NDK_HOME%;%ANDROID_HOME%\platform-tools;%PATH%"
ndk-build.cmd
