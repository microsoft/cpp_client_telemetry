//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

public class PrivacyGuardInitConfig {
    private var odwPrivacyGuardInitConfig: ODWPrivacyGuardInitConfig

    /**
    Constructor which inits with the `ODWPrivacyGuardInitConfig` object.

    - Parameters:
        - privacyGuardInitConfig: `ODWPrivacyGuardInitConfig` object which would be wrapped around.
    */
    public init(privacyGuardInitConfig: ODWPrivacyGuardInitConfig) {
        self.odwPrivacyGuardInitConfig = privacyGuardInitConfig
    }

    /// Returns the `ODWPrivacyGuardInitConfig` representing the corresponding ObjC object.
    public func getODWPrivacyGuardInitConfig() -> ODWPrivacyGuardInitConfig {
        return self.odwPrivacyGuardInitConfig
    }
}