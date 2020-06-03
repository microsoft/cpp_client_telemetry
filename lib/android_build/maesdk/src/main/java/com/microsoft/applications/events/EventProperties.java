package com.microsoft.applications.events;

public class EventProperties {
    public final long eventPropertiesPointer; // using the "store pointers as longs" convention

    public EventProperties(String name) {
        eventPropertiesPointer = EventPropertiesConstruct(name);
    };
    private static native long EventPropertiesConstruct(String name);

    public static native boolean SetName(long reference, String name);

    public static native boolean SetType(long reference, String type);

    public static native void SetPriority(long reference, int priority);

    public static native void SetLatency(long reference, int latency);

    public static native void SetPersistence(long reference, int persistence);

    public static native void SetProperty(long eventPropertiesRef, String name, long eventPropertyRef);
}
