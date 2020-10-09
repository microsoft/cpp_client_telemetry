//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The NetworkType enumeration contains a set of values that specify the type of network that a device is connected to.
 */
public enum NetworkType {
    /**
     * Any type of network.
     */
    Any(-1),

    /**
     * The type of network is unknown.
     */
    Unknown(0),

    /**
     * A wired network.
     */
    Wired(1),

    /**
     * A Wi-fi network.
     */
    Wifi(2),

    /**
     * A wireless wide-area network.
     */
    WWAN(3);

    private final int m_value;

    NetworkType(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

