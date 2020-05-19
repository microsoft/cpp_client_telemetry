package com.microsoft.applications.events;

class EventPropertyStringArrayValue extends EventPropertyValue{
    private String[] m_value;

    public EventPropertyStringArrayValue(final String[] value) {
        super(EventPropertyType.TYPE_STRING_ARRAY);
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");
        m_value = value;
    }

    @Override
    public String[] getStringArray() {
        return m_value;
    }
}
