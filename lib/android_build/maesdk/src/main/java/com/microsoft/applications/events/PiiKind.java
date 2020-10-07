//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The PiiKind enumeration contains a set of values that specify the kind of PII (Personal Identifiable Information) for tagging an event property
 */
public enum PiiKind {
    /**
     * No PII kind.
     */
    None(0),

    /**
     * An LDAP distinguished name.
     */
    DistinguishedName(1),

    /**
     * Generic data.
     */
    GenericData(2),

    /**
     * An IPV4 Internet address.
     */
    IPv4Address(3),

    /**
     * An IPV6 Internet address.
     */
    IPv6Address(4),

    /**
     * An e-mail subject.
     */
    MailSubject(5),

    /**
     * A telephone number.
     */
    PhoneNumber(6),

    /**
     * A query string.
     */
    QueryString(7),

    /**
     * A SIP address
     */
    SipAddress(8),

    /**
     * An e-mail address.
     */
    SmtpAddress(9),

    /**
     * An identity.
     */
    Identity(10),

    /**
     * A uniform resource indicator.
     */
    Uri(11),

    /**
     * A fully-qualified domain name.
     */
    Fqdn(12),

    /**
     * A legacy IPV4 Internet address.
     */
    IPv4AddressLegacy(13),

    CustomerContentKind_GenericData(32);

    private final int m_value;

    PiiKind(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}

