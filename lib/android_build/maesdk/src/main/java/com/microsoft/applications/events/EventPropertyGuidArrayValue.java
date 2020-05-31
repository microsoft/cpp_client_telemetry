package com.microsoft.applications.events;

import java.util.UUID;

class EventPropertyGuidArrayValue extends EventPropertyValue {
    private String[] m_value;

    public EventPropertyGuidArrayValue(final UUID[] value) {
        super(EventPropertyType.TYPE_GUID_ARRAY);
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");

        m_value = new String[value.length];
        for(int i = 0; i < value.length; i++) {
            m_value[i] = value[i].toString();
        }
    }

    @Override
    public String[] getGuidArray() {
        return m_value;
    }
}
