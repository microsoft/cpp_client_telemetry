package com.microsoft.applications.events;

public enum NetworkType {
    /// <summary>Any type of network.</summary>
    NetworkType_Any(1),
    /// <summary>The type of network is unknown.</summary>
    NetworkType_Unknown(0),
    /// <summary>A wired network.</summary>
    NetworkType_Wired(1),
    /// <summary>A Wi-fi network.</summary>
    NetworkType_Wifi(2),
    /// <summary>A wireless wide-area network.</summary>
    NetworkType_WWAN(3);

    private final int m_value;

    private NetworkType(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
