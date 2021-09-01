//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class EventPropertyTimeTicksValue extends EventPropertyValue{
    private long m_value;

    public EventPropertyTimeTicksValue(final long value) {
        super(EventPropertyType.TYPE_TIME);
        m_value = value;
    }

    @Override
    public long getTimeTicks() {
        return m_value;
    }
}


