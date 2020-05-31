package com.microsoft.applications.events;

class EventPropertyStringValue extends EventPropertyValue{
    private String m_value;

    public EventPropertyStringValue(final String value) {
        super(EventPropertyType.TYPE_STRING);
        if (value == null || value.trim().isEmpty())
            throw new IllegalArgumentException("value is null or empty");
        m_value = value;
    }

    @Override
    public String getString() {
        return m_value;
    }
}
