package com.microsoft.applications.events;

public enum PiiKind {
    /// <summary>No PII kind.</summary>
    None(0),
    /// <summary>An LDAP distinguished name.</summary>
    DistinguishedName(1),
    /// <summary>Generic data.</summary>
    GenericData(2),
    /// <summary>An IPV4 Internet address.</summary>
    IPv4Address(3),
    /// <summary>An IPV6 Internet address.</summary>
    IPv6Address(4),
    /// <summary>An e-mail subject.</summary>
    MailSubject(5),
    /// <summary>A telephone number.</summary>
    PhoneNumber(6),
    /// <summary>A query string.</summary>
    QueryString(7),
    /// <summary>A SIP address</summary>
    SipAddress(8),
    /// <summary>An e-mail address.</summary>
    SmtpAddress(9),
    /// <summary>An identity.</summary>
    Identity(10),
    /// <summary>A uniform resource indicator.</summary>
    Uri(11),
    /// <summary>A fully-qualified domain name.</summary>
    Fqdn(12),
    /// <summary>A legacy IPV4 Internet address.</summary>
    IPv4AddressLegacy(13),

    CustomerContentKind_GenericData(32);

    private final int m_value;

    private PiiKind(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
