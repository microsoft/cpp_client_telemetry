package com.microsoft.applications.events;

public enum PiiKind {
    /// <summary>No PII kind.</summary>
    PiiKind_None(0),
    /// <summary>An LDAP distinguished name.</summary>
    PiiKind_DistinguishedName(1),
    /// <summary>Generic data.</summary>
    PiiKind_GenericData(2),
    /// <summary>An IPV4 Internet address.</summary>
    PiiKind_IPv4Address(3),
    /// <summary>An IPV6 Internet address.</summary>
    PiiKind_IPv6Address(4),
    /// <summary>An e-mail subject.</summary>
    PiiKind_MailSubject(5),
    /// <summary>A telephone number.</summary>
    PiiKind_PhoneNumber(6),
    /// <summary>A query string.</summary>
    PiiKind_QueryString(7),
    /// <summary>A SIP address</summary>
    PiiKind_SipAddress(8),
    /// <summary>An e-mail address.</summary>
    PiiKind_SmtpAddress(9),
    /// <summary>An identity.</summary>
    PiiKind_Identity(10),
    /// <summary>A uniform resource indicator.</summary>
    PiiKind_Uri(11),
    /// <summary>A fully-qualified domain name.</summary>
    PiiKind_Fqdn(12),
    /// <summary>A legacy IPV4 Internet address.</summary>
    PiiKind_IPv4AddressLegacy(13),

    CustomerContentKind_GenericData(32);

    private final int m_value;

    private PiiKind(int value) {
        m_value = value;
    }

    public int getValue() {
        return m_value;
    }
}
