//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.util.UUID;

public class EventPropertyGuidArrayValue extends EventPropertyValue {
    private String[] m_value;

    public EventPropertyGuidArrayValue(final UUID[] value) {
        super(EventPropertyType.TYPE_GUID_ARRAY);
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");

        m_value = new String[value.length];
        for(int i = 0; i < value.length; i++) {
            UUID uuid = value[i];
            if (uuid == null)
                throw new IllegalArgumentException("UUID value is null for array index:" + i);

            m_value[i] = uuid.toString();
        }
    }

    @Override
    public String[] getGuidArray() {
        return m_value;
    }
}

