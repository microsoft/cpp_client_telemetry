// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

import java.util.HashMap;
import java.util.AbstractMap.SimpleEntry;

import android.util.Log;

/**
 * Class containing properties of one telemetry event.
 */
public class EventProperties {

    private static final String LOG_TAG = EventProperties.class.getSimpleName();

    protected String name = null;
    protected HashMap<String, HashMap.SimpleEntry<String, PiiKind>> properties = null;
    protected EventPriority priority = EventPriority.UNSPECIFIED;

    /**
     * Construct an EventProperties object. A non-empty name is required
     * whenever any custom properties are provided for the event.
     *
     * @param eventName Name of the event.
     */
    public EventProperties(String eventName) {
        Log.v(LOG_TAG, String.format("Creating EventProperties: %s", eventName));
        this.name = eventName;
        this.properties = new HashMap<String, HashMap.SimpleEntry<String, PiiKind>>();
    }

    /**
     * Set a property for the event.
     *
     * @param name    Name of the property.
     * @param value   Value of the property.
     * @param piiKind PII type of the property.
     * @see PiiKind
     */
    public void setProperty(String name, String value, PiiKind piiKind) {
        this.properties.put(name, new HashMap.SimpleEntry<String, PiiKind>(value, piiKind));
    }

    /**
     * [Optional] Set the priority for the event. Default transmit priority will
     * be used for transmitting the event if none was specified.
     *
     * @param priority EventPriority value.
     * @see EventPriority
     */
    public void setPriority(EventPriority priority) {
        Log.v(LOG_TAG, String.format("setPriority: %d", priority.getValue()));
        this.priority = priority;
    }
}
