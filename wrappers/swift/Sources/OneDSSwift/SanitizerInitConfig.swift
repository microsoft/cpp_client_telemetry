//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import ObjCModule

public final class SanitizerInitConfig {
    let odwSanitizerInitConfig: ODWSanitizerInitConfig

    /// Default constructor.
    public init() {
        odwSanitizerInitConfig = ODWSanitizerInitConfig()
    }

    /// (OPTIONAL) Custom event name to use when logging privacy concerns. Default value is `PrivacyConcern`.
    public var notificationEventName: String {
        get {
            odwSanitizerInitConfig.notificationEventName
        }
        set {
            odwSanitizerInitConfig.notificationEventName = newValue
        }
    }

    /**
    Returns the Obj-C object of the wrapper.

    - Return:
        `ODWSanitizerInitConfig` object which class is wrapped around.
    */
    func getODWSanitizerInitConfig() -> ODWSanitizerInitConfig {
        return odwSanitizerInitConfig
    }
}