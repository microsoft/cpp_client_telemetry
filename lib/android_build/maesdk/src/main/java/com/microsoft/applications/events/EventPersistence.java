//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * Persistence for an event
 */
public enum EventPersistence {
    /**
     * Normal
     */
    Normal(1),

    /**
     * Critical: priority upload and last to be evicted from offline storage
     */
    Critical(2),

    /**
     * DoNotStoreOnDisk: do not store event in offline storage
     */
    DoNotStoreOnDisk(3);

    private final int m_value;

    EventPersistence(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

