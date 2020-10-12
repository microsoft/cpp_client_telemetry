//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public enum DataCategory {
    /**
     * This is default transmission mode
     */
    PartC(0),

    PartB(1),

    MAX(2);

    private final int m_value;

    DataCategory(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}

