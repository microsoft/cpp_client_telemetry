package com.microsoft.applications.events;

class EventPropertyDoubleValue extends EventPropertyValue{
    private double m_value;

    public EventPropertyDoubleValue(final double value) {
        super(EventPropertyType.TYPE_DOUBLE);
        m_value = value;
    }

    @Override
    public double getDouble() {
        return m_value;
    }
}
