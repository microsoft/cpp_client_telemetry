//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The UserState enumeration contains a set of values that specify the user's state. For example, connected.
 */
public enum UserState {
    /**
     * User state unknown.
     */
    Unknown(0),

    /**
     * The user is connected to a service.
     */
    Connected(1),

    /**
     * The user is reachable for a service like push notification.
     */
    Reachable(2),

    /**
     * The user is signed-into a service.
     */
    SignedIn(3),

    /**
     * The user is signed-out of a service.
     */
    SignedOut(4);

    private final int m_value;

    UserState(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

