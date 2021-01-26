//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class EventPropertyLongValue extends EventPropertyValue{
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

