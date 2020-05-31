package com.microsoft.applications.events;

public enum EventPriority {
    /// <summary>The event priority is not specified.</summary>
    Unspecified(-1),
    /// <summary>The event will not be transmitted.</summary>
    Off(0),
    /// <summary>low priority.</summary>
    Low(1),
    /// <summary>Same as Low.</summary>
    MIN(Low.getValue()),
    /// <summary>Normal priority.</summary>
    Normal(2),
    /// <summary>High priority.</summary>
    High(3),
    /// <summary>The event will be transmitted as soon as possible.</summary>
    Immediate(4),
    /// <summary>Same as Immediate.</summary>
    MAX(Immediate.getValue());

    static final int VALUE_UNSPECIFIED = -1;
    static final int VALUE_OFF = 0;
    static final int VALUE_LOW = 1;
    static final int VALUE_NORMAL = 2;
    static final int VALUE_HIGH = 3;
    static final int VALUE_IMMEDIATE = 4;

    private final int m_value;

    private EventPriority(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }

    public static EventPriority getEnum(int value) {
        switch (value) {
            case VALUE_UNSPECIFIED :
                return Unspecified;
            case VALUE_OFF :
                return Off;
            case VALUE_LOW :
                return Low;
            case VALUE_NORMAL :
                return Normal;
            case VALUE_HIGH :
                return High;
            case VALUE_IMMEDIATE :
                return Immediate;
            default :
                throw new IllegalArgumentException("Unsupported value: " + value);
        }
    }
}
