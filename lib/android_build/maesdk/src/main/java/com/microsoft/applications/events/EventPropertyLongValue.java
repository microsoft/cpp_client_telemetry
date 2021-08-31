//
// Copyright (c) Microsoft Corporation. All rights reserved.
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

