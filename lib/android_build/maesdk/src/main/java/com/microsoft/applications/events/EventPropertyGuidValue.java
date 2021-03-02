//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.util.UUID;

public class EventPropertyGuidValue extends EventPropertyValue{
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


