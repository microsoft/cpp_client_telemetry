//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class EventPropertyStringValue extends EventPropertyValue{
    private String m_value;

    public EventPropertyStringValue(final String value) {
        super(EventPropertyType.TYPE_STRING);
        if (value == null)
            throw new IllegalArgumentException("value is null");
        m_value = value;
    }

    @Override
    public String getString() {
        return m_value;
    }
}

