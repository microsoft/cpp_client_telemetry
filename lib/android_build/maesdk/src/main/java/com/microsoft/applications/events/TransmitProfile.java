//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * Transmit profiles to choose from for event transmission that could favor low transmission
 * latency or device resource consumption.
 */
public enum TransmitProfile {
    /**
     * Favors low transmission latency, but may consume more data bandwidth and power.
     */
    RealTime(0),

    /**
     * Favors near real-time transmission latency. Automatically balances
     * transmission latency with data bandwidth and power consumption.
     */
    NearRealTime(1),

    /**
     * Favors device performance by conserving both data bandwidth and power consumption.
     */
    BestEffort(2);

    private final int m_value;

    TransmitProfile(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

