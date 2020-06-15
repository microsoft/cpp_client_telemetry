package com.microsoft.applications.events;

class EventPropertyBooleanValue extends EventPropertyValue{
    private boolean m_value;

    public EventPropertyBooleanValue(final boolean value) {
        super(EventPropertyType.TYPE_BOOLEAN);
        m_value = value;
    }

    @Override
    public boolean getBoolean() {
        return m_value;
    }
}
