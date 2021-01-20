//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;

@Keep
public abstract class EventPropertyValue {
    private EventPropertyType m_type;

    EventPropertyValue(EventPropertyType type) {
        m_type = type;
    }

    public int getType() {
        return m_type.getValue();
    }

    public String getString()          { throw new java.lang.UnsupportedOperationException(); }
    public long getLong()              { throw new java.lang.UnsupportedOperationException(); }
    public double getDouble()          { throw new java.lang.UnsupportedOperationException(); }
    public boolean getBoolean()        { throw new java.lang.UnsupportedOperationException(); }
    public String getGuid()            { throw new java.lang.UnsupportedOperationException(); }
    public long getTimeTicks()         { throw new java.lang.UnsupportedOperationException(); }

    public String[] getStringArray()   { throw new java.lang.UnsupportedOperationException(); }
    public long[] getLongArray()       { throw new java.lang.UnsupportedOperationException(); }
    public double[] getDoubleArray()   { throw new java.lang.UnsupportedOperationException(); }
    public String[] getGuidArray()     { throw new java.lang.UnsupportedOperationException(); }
}

