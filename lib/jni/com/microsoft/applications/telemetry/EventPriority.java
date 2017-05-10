// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

/**
 * Supported event priorities.
 * These values correspond to those defined in the C++ SDK API.
 */
public enum EventPriority {

    UNSPECIFIED(-1), OFF(0), LOW(1), NORMAL(2), HIGH(3), IMMEDIATE(4);

    private final int val;

    private EventPriority(int value) {
        val = value;
    }

    public int getValue() {
        return val;
    }

    /**
     * Create an EventPriority enum from the given integer value.
     *
     * @param value the integer value corresponding to the priority
     * @return an event priority based on the given value
     */
    public static EventPriority fromValue(int value) {
        switch (value) {
            case -1:
                return EventPriority.UNSPECIFIED;
            case 0:
                return EventPriority.OFF;
            case 1:
                return EventPriority.LOW;
            case 2:
                return EventPriority.NORMAL;
            case 3:
                return EventPriority.HIGH;
            case 4:
                return EventPriority.IMMEDIATE;
            default:
                throw new IllegalArgumentException("Unknown EventPriority value: " + value);
        }
    }
}
