//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The AppLifecycleState enumeration contains a set of values that specify the lifecycle state of the application.
 */
public enum AppLifecycleState {
    /**
     * Lifecycle state unknown.
     */
    Unknown(0),
    /**
     * The application launched.
     */
    Launch(1),
    /**
     * The application exited.
     */
    Exit(2),
    /**
     * The application suspended.
     */
    Suspend(3),
    /**
     * The application resumed.
     */
    Resume(4),
    /**
     * The application came back into the foreground.
     */
    Foreground(5),
    /**
     * The application went into the background.
     */
    Background(6);

    private final int m_value;

    AppLifecycleState(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

