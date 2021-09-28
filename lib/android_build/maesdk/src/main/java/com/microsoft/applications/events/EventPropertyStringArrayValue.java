//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class EventPropertyStringArrayValue extends EventPropertyValue{
    private String[] m_value;

    public EventPropertyStringArrayValue(final String[] value) {
        super(EventPropertyType.TYPE_STRING_ARRAY);
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");

        m_value = new String[value.length];
        for(int i = 0; i < value.length; i++) {
            String strValue = value[i];
            if (strValue == null)
                throw new IllegalArgumentException("String value is null for array index:" + i);

            m_value[i] = strValue;
        }
    }

    @Override
    public String[] getStringArray() {
        return m_value;
    }
}

