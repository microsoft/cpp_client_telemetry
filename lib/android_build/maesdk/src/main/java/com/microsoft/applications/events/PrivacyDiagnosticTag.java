//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * Diagnostic tags for privacy tagging of the events.
 */
public enum PrivacyDiagnosticTag {
    BrowsingHistory(0x0000000000000002L),

    DeviceConnectivityAndConfiguration(0x0000000000000800L),

    InkingTypingAndSpeechUtterance(0x0000000000020000L),

    ProductAndServicePerformance(0x0000000001000000L),

    ProductAndServiceUsage(0x0000000002000000L),

    SoftwareSetupAndInventory(0x0000000080000000L);

    private final long m_value;

    PrivacyDiagnosticTag(long value) {
        m_value = value;
    }

    public long getValue() {
        return m_value;
    }
}

