//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * Microsoft Office diagnostic level classification
 */
public enum DiagnosticLevel {
    /**
     * Data that we need to collect in order to keep the product secure, up to date, and performing as expected
     */
    DIAG_LEVEL_REQUIRED(1),

    /**
     * Additional optional data
     */
    DIAG_LEVEL_OPTIONAL(2),

    /**
     * Data required for services to be able to function properly
     */
    DIAG_LEVEL_RSD(110),

    /**
     * Data required for operation of essential services such as licensing, etc.
     */
    DIAG_LEVEL_RSDES(120);

    private final int m_value;

    DiagnosticLevel(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

