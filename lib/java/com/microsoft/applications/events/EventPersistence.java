package com.microsoft.applications.events;

public enum EventPersistence {

    /// Normal
    Normal(1),
    /// Critical: priority upload and last to be evicted from offline storage
    Critical(2),
    /// DoNotStoreOnDisk: do not store event in offline storage
    DoNotStoreOnDisk(3);

    private final int m_value;

    private EventPersistence(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
