//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import ObjCModule

public final class PrivacyGuardInitConfig {
    let odwPrivacyGuardInitConfig: ODWPrivacyGuardInitConfig
    private var commonDataContext: CommonDataContext

    /// Data Context to use with the Privacy Guard.
    public var dataContext: CommonDataContext {
        get {
            commonDataContext
        }
        set {
            commonDataContext = newValue
            odwPrivacyGuardInitConfig.dataContext = commonDataContext.odwCommonDataContext
        }
    }

    /// (OPTIONAL) Custom event name to use when logging privacy concerns. Default value is `PrivacyConcern`.
    public var notificationEventName: String {
        get {
            odwPrivacyGuardInitConfig.notificationEventName
        }
        set {
            odwPrivacyGuardInitConfig.notificationEventName = newValue
        }
    }

    /// (OPTIONAL) Custom event name to use when logging concerns identified in the Semantic Context. Default value is `SemanticContext`.
    public var semanticContextNotificationEventName: String {
        get {
            odwPrivacyGuardInitConfig.semanticContextNotificationEventName
        }
        set {
            odwPrivacyGuardInitConfig.semanticContextNotificationEventName = newValue
        }
    }

    /// (OPTIONAL) Custom event name to use when logging summary events. Default value is `PrivacyGuardSummary`.
    public var summaryEventName: String {
        get {
            odwPrivacyGuardInitConfig.summaryEventName
        }
        set {
            odwPrivacyGuardInitConfig.summaryEventName = newValue
        }
    }

    /// (OPTIONAL) Add `PG_` prefix to Notification and Summary event field names. Default value is `false`.
    public var useEventFieldPrefix: Bool {
        get {
            odwPrivacyGuardInitConfig.useEventFieldPrefix
        }
        set {
            odwPrivacyGuardInitConfig.useEventFieldPrefix = newValue
        }
    }

    /// (OPTIONAL) Should scan for URLs? Default value is `true`.
    public var scanForUrls: Bool {
        get {
            odwPrivacyGuardInitConfig.scanForUrls
        }
        set {
            odwPrivacyGuardInitConfig.scanForUrls = newValue
        }
    }

    /// (OPTIONAL) Should disable advanced scans such as location, URLs, Out-of-scope identifiers, etc.
    public var disableAdvancedScans: Bool {
        get {
            odwPrivacyGuardInitConfig.disableAdvancedScans
        }
        set {
            odwPrivacyGuardInitConfig.disableAdvancedScans = newValue
        }
    }

    /// Default constructor.
    public init() {
        odwPrivacyGuardInitConfig = ODWPrivacyGuardInitConfig()
        odwPrivacyGuardInitConfig.dataContext = ODWCommonDataContext()
        commonDataContext = CommonDataContext(odwCommonDataContext: odwPrivacyGuardInitConfig.dataContext)
    }

    /**
    Returns the Obj-C object of the wrapper.

    - Return:
        `ODWPrivacyGuardInitConfig` object which class is wrapped around.
    */
    func getODWPrivacyGuardInitConfig() -> ODWPrivacyGuardInitConfig {
        return odwPrivacyGuardInitConfig
    }
}
