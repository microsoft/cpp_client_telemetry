package com.microsoft.applications.events;

import java.util.UUID;

class EventPropertyGuidValue extends EventPropertyValue{
    private String m_value;

    public EventPropertyGuidValue(final UUID value) {
        super(EventPropertyType.TYPE_GUID);
        if (value == null)
            throw new IllegalArgumentException("value is null");
        m_value = value.toString();
    }

    @Override
    public String getGuid() {
        return m_value;
    }
}

