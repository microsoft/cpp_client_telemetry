package com.microsoft.applications.events;

public enum TransmitProfile {
    /// <summary>Favors low transmission latency, but may consume more data bandwidth and power.</summary>
    TransmitProfile_RealTime(0),
    /// <summary>Favors near real-time transmission latency. Automatically balances transmission
    /// latency with data bandwidth and power consumption.</summary>
    TransmitProfile_NearRealTime(1),
    /// <summary>Favors device performance by conserving both data bandwidth and power consumption.</summary>
    TransmitProfile_BestEffort(2);

    private final int m_value;

    private TransmitProfile(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
