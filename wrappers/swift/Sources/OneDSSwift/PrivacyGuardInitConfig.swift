//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import ObjCModule

public final class PrivacyGuardInitConfig {
    public let odwPrivacyGuardInitConfig: ODWPrivacyGuardInitConfig

    /**
    Constructor which inits with the `ODWPrivacyGuardInitConfig` object.

    - Parameters:
        - privacyGuardInitConfig: `ODWPrivacyGuardInitConfig` object which would be wrapped around.
    */
    public init(privacyGuardInitConfig: ODWPrivacyGuardInitConfig) {
        self.odwPrivacyGuardInitConfig = privacyGuardInitConfig
    }

    /**
    Returns the Obj-C object of the wrapper.

    - Return:
        `ODWPrivacyGuardInitConfig` object which class is wrapped around.
    */
    public func getODWPrivacyGuardInitConfig() -> ODWPrivacyGuardInitConfig {
        return odwPrivacyGuardInitConfig
    }
}