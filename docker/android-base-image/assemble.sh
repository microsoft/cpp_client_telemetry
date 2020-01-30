#!/bin/sh -x
cd ${GITHUB_WORKSPACE}/lib/android_build
chmod +x gradlew
which cmake
echo $PATH
./gradlew assemble || exit 1
tar cvf ${GITHUB_WORKSPACE}/artifacts.tar ariasdk/build/outputs/aar ariasdk/build/intermediates/merged_native_libs
