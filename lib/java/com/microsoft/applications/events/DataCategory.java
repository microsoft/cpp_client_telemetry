package com.microsoft.applications.events;

public enum DataCategory {
    DataCategory_PartC(0), //This is default transmission mode
    DataCategory_PartB(1),
    DataCategory_MAX(2);

    private final int m_value;

    private DataCategory(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
