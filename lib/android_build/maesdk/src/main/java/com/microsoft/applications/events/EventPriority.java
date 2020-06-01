package com.microsoft.applications.events;

/**
 * The EventPriority enumeration contains a set of values that specify the priority for an event.
 */
public enum EventPriority {
    /**
     * The event priority is not specified.
     */
    Unspecified(-1),

    /**
     * The event will not be transmitted.
     */
    Off(0),

    /**
     * low priority.
     */
    Low(1),

    /**
     * Same as Low.
     */
    MIN(Low.getValue()),

    /**
     * Normal priority.
     */
    Normal(2),

    /**
     * High priority.
     */
    High(3),

    /**
     * The event will be transmitted as soon as possible.
     */
    Immediate(4),

    /**
     * Same as Immediate.
     */
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
