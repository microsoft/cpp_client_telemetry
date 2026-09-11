-keep interface com.microsoft.applications.events.IDataViewer { *; }
-keep class * implements com.microsoft.applications.events.IDataViewer {
    public *;
}