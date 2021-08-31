//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class EventPropertyBooleanValue extends EventPropertyValue{
    private boolean m_value;

    public EventPropertyBooleanValue(final boolean value) {
        super(EventPropertyType.TYPE_BOOLEAN);
        m_value = value;
    }

    @Override
    public boolean getBoolean() {
        return m_value;
    }
}

