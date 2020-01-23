package com.microsoft.applications.events;

class EventPropertyValue {
    private EventPropertyType m_type;

    EventPropertyValue(EventPropertyType type) {
        m_type = type;
    }

    EventPropertyType getType() {
        return m_type;
    }

    String getString()          { throw new java.lang.UnsupportedOperationException(); }
    long getLong()              { throw new java.lang.UnsupportedOperationException(); }
    double getDouble()          { throw new java.lang.UnsupportedOperationException(); }
    boolean getBoolean()        { throw new java.lang.UnsupportedOperationException(); }
    String getGuid()            { throw new java.lang.UnsupportedOperationException(); }
    long getTimeTicks()         { throw new java.lang.UnsupportedOperationException(); }

    String[] getStringArray()   { throw new java.lang.UnsupportedOperationException(); }
    long[] getLongArray()       { throw new java.lang.UnsupportedOperationException(); }
    double[] getDoubleArray()   { throw new java.lang.UnsupportedOperationException(); }
    String[] getGuidArray()     { throw new java.lang.UnsupportedOperationException(); }
}
