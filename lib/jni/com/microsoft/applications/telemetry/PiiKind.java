// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

/**
 * Supported Personally Identifiable Information types.
 * These values correspond to those defined in the C++ SDK API.
 */
public enum PiiKind {
    NONE(0), DISTINGUISHED_NAME(1), GENERIC_DATA(2), IPV4_ADDRESS(3),
    IPV6_ADDRESS(4), MAIL_SUBJECT(5), PHONE_NUMBER(6), QUERY_STRING(7),
    SIP_ADDRESS(8), SMTP_ADDRESS(9), IDENTITY(10), URI(11), FQDN(12),
    IPV4_ADDRESS_LEGACY(13);

    private final int val;

    private PiiKind(int value) {
        val = value;
    }

    public int getValue() {
        return val;
    }
}
