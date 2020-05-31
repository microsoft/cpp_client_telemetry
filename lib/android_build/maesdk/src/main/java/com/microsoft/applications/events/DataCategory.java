package com.microsoft.applications.events;

public enum DataCategory {
    PartC(0), //This is default transmission mode
    PartB(1),
    MAX(2);

    private final int m_value;

    private DataCategory(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
