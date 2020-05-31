package com.microsoft.applications.events;

public enum EventLatency {

    /// Unspecified: Event Latency is not specified
    Unspecified(-1),
    /// Off: Latency is not to be transmitted
    Off(0),
    /// Normal: Latency is to be transmitted at low priority
    Normal(1),
    /// Cost Deffered: Latency is to be transmitted at cost deferred priority
    CostDeferred(2),
    /// RealTime: Latency is to be transmitted at real time priority
    RealTime(3),
    /// Max: Latency is to be transmitted as soon as possible
    Max(4);

    static final int VALUE_UNSPECIFIED = -1;
    static final int VALUE_OFF = 0;
    static final int VALUE_NORMAL = 1;
    static final int VALUE_COST_DEFERRED = 2;
    static final int VALUE_REAL_TIME = 3;
    static final int VALUE_MAX = 4;

    private final int m_value;

    private EventLatency(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }

    public static EventLatency getEnum(int value) {
        switch (value) {
            case VALUE_UNSPECIFIED :
                return Unspecified;
            case VALUE_OFF :
                return Off;
            case VALUE_NORMAL :
                return Normal;
            case VALUE_COST_DEFERRED :
                return CostDeferred;
            case VALUE_REAL_TIME :
                return RealTime;
            case VALUE_MAX :
                return Max;
            default :
                throw new IllegalArgumentException("Unsupported value: " + value);
        }
    }
}
