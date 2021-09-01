//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

class EventPriorityValues {
    static final int VALUE_UNSPECIFIED = -1;
    static final int VALUE_OFF = 0;
    static final int VALUE_LOW = 1;
    static final int VALUE_NORMAL = 2;
    static final int VALUE_HIGH = 3;
    static final int VALUE_IMMEDIATE = 4;
}

/**
 * The EventPriority enumeration contains a set of values that specify the priority for an event.
 */
public enum EventPriority {
    /**
     * The event priority is not specified.
     */
    Unspecified(EventPriorityValues.VALUE_UNSPECIFIED),

    /**
     * The event will not be transmitted.
     */
    Off(EventPriorityValues.VALUE_OFF),

    /**
     * low priority.
     */
    Low(EventPriorityValues.VALUE_LOW),

    /**
     * Same as Low.
     */
    MIN(Low.getValue()),

    /**
     * Normal priority.
     */
    Normal(EventPriorityValues.VALUE_NORMAL),

    /**
     * High priority.
     */
    High(EventPriorityValues.VALUE_HIGH),

    /**
     * The event will be transmitted as soon as possible.
     */
    Immediate(EventPriorityValues.VALUE_IMMEDIATE),

    /**
     * Same as Immediate.
     */
    MAX(Immediate.getValue());

    private final int m_value;

    EventPriority(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }

    static EventPriority getEnum(int value) {
        switch (value) {
            case EventPriorityValues.VALUE_UNSPECIFIED :
                return Unspecified;
            case EventPriorityValues.VALUE_OFF :
                return Off;
            case EventPriorityValues.VALUE_LOW :
                return Low;
            case EventPriorityValues.VALUE_NORMAL :
                return Normal;
            case EventPriorityValues.VALUE_HIGH :
                return High;
            case EventPriorityValues.VALUE_IMMEDIATE :
                return Immediate;
            default :
                throw new IllegalArgumentException("Unsupported value: " + value);
        }
    }
}

