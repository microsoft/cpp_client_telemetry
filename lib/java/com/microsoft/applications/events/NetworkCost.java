package com.microsoft.applications.events;

public enum NetworkCost {
    /// <summary>Any network cost.</summary>
    NetworkCost_Any(1),
    /// <summary>Network cost unknown.</summary>
    NetworkCost_Unknown(0),
    /// <summary>Unmetered.</summary>
    NetworkCost_Unmetered(1),
    /// <summary>Metered.</summary>
    NetworkCost_Metered(2),
    /// <summary>The device is roaming.</summary>
    NetworkCost_Roaming(3);

    private final int m_value;

    private NetworkCost(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}

