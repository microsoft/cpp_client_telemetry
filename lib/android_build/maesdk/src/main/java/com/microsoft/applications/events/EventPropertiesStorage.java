package com.microsoft.applications.events;

import java.util.HashMap;
import java.util.Map;

class EventPropertiesStorage {
    public String              eventName = "";
    public String              eventType = "";
    public EventLatency        eventLatency = EventLatency.Normal;
    public EventPersistence    eventPersistence = EventPersistence.Normal;
    public double              eventPopSample = 100;
    public long                eventPolicyBitflags = 0;
    public long                timestampInMillis = 0;

    public Map<String, EventProperty> properties = new HashMap<>();
    public Map<String, EventProperty> propertiesPartB = new HashMap<>();

    public void copyEventPropertiesStorage(EventPropertiesStorage other) {
        if (other == null)
            throw new IllegalArgumentException("other is null");

        eventName = other.eventName;
        eventType = other.eventType;
        eventLatency = other.eventLatency;
        eventPersistence = other.eventPersistence;
        eventPopSample = other.eventPopSample;
        eventPolicyBitflags = other.eventPolicyBitflags;
        timestampInMillis = other.timestampInMillis;

        if (properties == null) {
            properties = other.properties;
        }
        else {
            properties.putAll(other.properties);
        }
    }

    public void addProperties(Map<String, EventProperty> properties) {
        if (properties == null)
            throw new IllegalArgumentException("properties is null");

        this.properties.putAll(properties);
    }

    public void assignProperties(Map<String, EventProperty> properties) {
        if (properties == null)
            throw new IllegalArgumentException("properties is null");

        this.properties = properties;
    }
}
