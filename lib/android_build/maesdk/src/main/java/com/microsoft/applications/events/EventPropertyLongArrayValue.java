package com.microsoft.applications.events;

class EventPropertyLongArrayValue extends EventPropertyValue{
    private long[] m_value;

    public EventPropertyLongArrayValue(final long[] value) {
        super(EventPropertyType.TYPE_LONG_ARRAY);
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");
        m_value = value;
    }

    @Override
    public long[] getLongArray() {
        return m_value;
    }
}

