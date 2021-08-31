//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class EventPropertyDoubleArrayValue extends EventPropertyValue{
    private double[] m_value;

    public EventPropertyDoubleArrayValue(final double[] value) {
        super(EventPropertyType.TYPE_DOUBLE_ARRAY);
        if (value == null || value.length == 0)
            throw new IllegalArgumentException("value is null or empty");
        m_value = value;
    }

    @Override
    public double[] getDoubleArray() {
        return m_value;
    }
}

