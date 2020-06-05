package com.microsoft.applications.events;

public class LoggerWrapper {

    public static void InitializeLoggerWrapper() {
        pLoggerWrapper = Initialize();
    }

    public static void LogEvent(EventProperties eventProperties) {
        LogEvent(eventProperties.eventPropertiesPointer, pLoggerWrapper);
    }

    private static native void LogEvent(long pEventProperties, long pLoggerWrapper);

    private static native long Initialize();

    private static long pLoggerWrapper;
}
