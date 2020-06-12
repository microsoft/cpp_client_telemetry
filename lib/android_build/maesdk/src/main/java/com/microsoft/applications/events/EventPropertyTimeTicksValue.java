package com.microsoft.applications.events;

class EventPropertyTimeTicksValue extends EventPropertyValue{
    private long m_value;

    public EventPropertyTimeTicksValue(final long value) {
        super(EventPropertyType.TYPE_TIME);
        m_value = value;
    }

    @Override
    public long getTimeTicks() {
        return m_value;
    }
}

