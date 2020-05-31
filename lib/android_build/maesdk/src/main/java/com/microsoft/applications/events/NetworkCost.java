package com.microsoft.applications.events;

public enum NetworkCost {
    /// <summary>Any network cost.</summary>
    Any(1),
    /// <summary>Network cost unknown.</summary>
    Unknown(0),
    /// <summary>Unmetered.</summary>
    Unmetered(1),
    /// <summary>Metered.</summary>
    Metered(2),
    /// <summary>The device is roaming.</summary>
    Roaming(3);

    private final int m_value;

    private NetworkCost(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}

