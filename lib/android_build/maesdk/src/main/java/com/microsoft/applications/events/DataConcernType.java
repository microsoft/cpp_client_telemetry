//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * The Data Concern Type enum lists the known concerns that the Privacy Guard Service
 * is able to detect and notify on. The most up-to-date list is available here: https://aka.ms/privacyguard/issuetypes
 */
public enum DataConcernType {
    /**
     * DefaultValue
     */
    None(0),
    /**
     *  Formatted text: HTML, MIME, RTF, Xml, etc.
     */
    Content(1),
    /**
     *  Country/region
     */
    DemographicInfoCountryRegion(2),
    /**
     *  The users language ID. Example: En-Us
     */
    DemographicInfoLanguage(3),
    /**
     *  Any directory or file share
     */
    Directory(4),
    /**
     *  SMTP not ending in <span>microsoft.com</span>
     */
    ExternalEmailAddress(5),
    /**
     *  Field name sounds like location data
     */
    FieldNameImpliesLocation(6),
    /**
     *  A file extension from the reportable list of extensions (ignores code files)
     */
    FileNameOrExtension(7),
    /**
     *  A URL referencing a common file-sharing site or service.
     */
    FileSharingUrl(8),
    /**
     *  EUPI. Any authenticated identifier of the same types used for DSR.
     */
    InScopeIdentifier(9),
    /**
     *  The current users EUPI for DSR
     */
    InScopeIdentifierActiveUser(10),
    /**
     *  SMTP ending with <span>microsoft.com</span>
     */
    InternalEmailAddress(11),
    /**
     *  Machine's current IP address
     */
    IpAddress(12),
    /**
     *  Data appears to specify a location in the real world
     */
    Location(13),
    /**
     *  Machine name
     */
    MachineName(14),
    /**
     *  Client Id for OXO telemetry from the registry
     */
    OutOfScopeIdentifier(15),
    /**
     *  Product key
     */
    PIDKey(16),
    /**
     *  A URL containing parameters "access_token", "password", etc.
     */
    Security(17),
    /**
     *  Any URL
     */
    Url(18),
    /**
     *  Current user's alias
     */
    UserAlias(19),
    /**
     *  User/Machine domain
     */
    UserDomain(20),
    /**
     *  Current user's name or part of it.
     */
    UserName(21);

    private final int m_value;

    DataConcernType(int value) {
        m_value = value;
    }

    int getValue() {
        return m_value;
    }
}

