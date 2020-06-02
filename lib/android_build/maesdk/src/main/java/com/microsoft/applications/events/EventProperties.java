package com.microsoft.applications.events;

public class EventProperties {
    public final long eventPropertiesPointer; // using the "store pointers as longs" convention

    public EventProperties(String name) {
        eventPropertiesPointer = EventPropertiesConstruct(name);
    };
    private static native long EventPropertiesConstruct(String name);

    public static native boolean SetName(long reference, String name);
}
