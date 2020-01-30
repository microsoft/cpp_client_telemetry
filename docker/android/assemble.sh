#!/bin/sh
cd lib/android_build
chmod +x gradlew
./gradlew assemble
tar cvf /artifacts/artifacts.tar ariasdk/build/outputs/aar ariasdk/build/intermediates/merged_native_libs
