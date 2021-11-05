call .\gradlew.bat app:connectedDebugAndroidTest --refresh-dependencies
adb.exe logcat -t 2000 MAE:D '*:E' > logcat.txt
