//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

class EventLatencyValues {
    static final int VALUE_UNSPECIFIED = -1;
    static final int VALUE_OFF = 0;
    static final int VALUE_NORMAL = 1;
    static final int VALUE_COST_DEFERRED = 2;
    static final int VALUE_REAL_TIME = 3;
    static final int VALUE_MAX = 4;
}

/**
 * Latency for an event to be transmitted
 */
public enum EventLatency {
    /**
     * Unspecified: Event Latency is not specified
     */
    Unspecified(EventLatencyValues.VALUE_UNSPECIFIED),

    /**
     * Off: Latency is not to be transmitted
     */
    Off(EventLatencyValues.VALUE_OFF),

    /**
     * Normal: Latency is to be transmitted at low priority
     */
    Normal(EventLatencyValues.VALUE_NORMAL),

    /**
     * Cost Deffered: Latency is to be transmitted at cost deferred priority
     */
    CostDeferred(EventLatencyValues.VALUE_COST_DEFERRED),

    /**
     * RealTime: Latency is to be transmitted at real time priority
     */
    RealTime(EventLatencyValues.VALUE_REAL_TIME),

    /**
     * Max: Latency is to be transmitted as soon as possible
     */
    Max(EventLatencyValues.VALUE_MAX);

    private final int m_value;

    EventLatency(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }

    static EventLatency getEnum(int value) {
        switch (value) {
            case EventLatencyValues.VALUE_UNSPECIFIED :
                return Unspecified;
            case EventLatencyValues.VALUE_OFF :
                return Off;
            case EventLatencyValues.VALUE_NORMAL :
                return Normal;
            case EventLatencyValues.VALUE_COST_DEFERRED :
                return CostDeferred;
            case EventLatencyValues.VALUE_REAL_TIME :
                return RealTime;
            case EventLatencyValues.VALUE_MAX :
                return Max;
            default :
                throw new IllegalArgumentException("Unsupported value: " + value);
        }
    }
}

