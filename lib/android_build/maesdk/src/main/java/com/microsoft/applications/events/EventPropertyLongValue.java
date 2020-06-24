package com.microsoft.applications.events;

class EventPropertyLongValue extends EventPropertyValue{
    private long m_value;

    public EventPropertyLongValue(final long value) {
        super(EventPropertyType.TYPE_LONG);
        m_value = value;
    }

    @Override
    public long getLong() {
        return m_value;
    }
}
