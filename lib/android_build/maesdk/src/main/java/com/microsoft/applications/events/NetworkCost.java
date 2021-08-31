//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The NetworkCost enumeration contains a set of values that specify the kind of network cost for a connected device.
 */
public enum NetworkCost {
    /**
     * Any network cost.
     */
    Any(1),

    /**
     * Network cost unknown.
     */
    Unknown(0),

    /**
     * Unmetered.
     */
    Unmetered(1),

    /**
     * Metered.
     */
    Metered(2),

    /**
     * The device is roaming.
     */
    Roaming(3);

    private final int m_value;

    NetworkCost(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}


