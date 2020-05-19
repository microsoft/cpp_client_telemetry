package com.microsoft.applications.events;

public enum NetworkType {
    /// <summary>Any type of network.</summary>
    Any(1),
    /// <summary>The type of network is unknown.</summary>
    Unknown(0),
    /// <summary>A wired network.</summary>
    Wired(1),
    /// <summary>A Wi-fi network.</summary>
    Wifi(2),
    /// <summary>A wireless wide-area network.</summary>
    WWAN(3);

    private final int m_value;

    private NetworkType(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
