package com.microsoft.applications.events;

public class EventProperty {
    public final long eventPropertyPointer;

    public EventProperty(String value) {
        eventPropertyPointer = EventPropertyConstruct(value);
    }

    private static native long EventPropertyConstruct(String value);
}