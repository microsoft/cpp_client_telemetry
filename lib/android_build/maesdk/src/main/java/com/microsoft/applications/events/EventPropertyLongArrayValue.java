//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class EventPropertyLongArrayValue extends EventPropertyValue{
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


