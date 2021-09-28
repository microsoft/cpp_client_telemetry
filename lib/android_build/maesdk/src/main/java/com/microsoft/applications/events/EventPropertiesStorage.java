//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.util.HashMap;
import java.util.Map;

class EventPropertiesStorage {
    String eventName;
    String eventType;
    EventLatency eventLatency;
    EventPersistence eventPersistence;
    double eventPopSample;
    long eventPolicyBitflags;
    long timestampInMillis;

    Map<String, EventProperty> properties;
    Map<String, EventProperty> propertiesPartB;

    EventPropertiesStorage() {
        eventName = "";
        eventLatency = EventLatency.Normal;
        eventPersistence = EventPersistence.Normal;
        eventPopSample = 100;
        eventPolicyBitflags = 0;
        timestampInMillis = 0;

        properties = new HashMap<>();
        propertiesPartB = new HashMap<>();
    }

    EventPropertiesStorage(EventPropertiesStorage other) {
        if (other == null)
            throw new IllegalArgumentException("other is null");

        eventName = other.eventName;
        eventType = other.eventType;
        eventLatency = other.eventLatency;
        eventPersistence = other.eventPersistence;
        eventPopSample = other.eventPopSample;
        eventPolicyBitflags = other.eventPolicyBitflags;
        timestampInMillis = other.timestampInMillis;
        properties = other.properties;
        propertiesPartB = other.propertiesPartB;
    }

    void addProperties(Map<String, EventProperty> properties) {
        if (properties == null)
            throw new IllegalArgumentException("properties is null");

        this.properties.putAll(properties);
    }

    void setProperties(Map<String, EventProperty> properties) {
        if (properties == null)
            throw new IllegalArgumentException("properties is null");

        this.properties = properties;
    }
}

