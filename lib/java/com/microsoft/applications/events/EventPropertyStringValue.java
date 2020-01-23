package com.microsoft.applications.events;

public class EventPropertyStringValue extends EventPropertyValue{
    private String m_value;

    EventPropertyStringValue(final String value) {
        super(EventPropertyType.STRING);
        m_value = value;
    }

    String getString() {
        return m_value;
    }
}
