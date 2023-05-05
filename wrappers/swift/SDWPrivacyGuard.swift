//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Wrapper to `ODWPrivacyGuard` representing Privacy Guard Hook.
public class PrivacyGuard {
    private var odwPrivacyGuard: ODWPrivacyGuard
    
    public init(privacyGuard: ODWPrivacyGuard) {
        self.odwPrivacyGuard = privacyGuard
    }

    // MARK: Behavior methods

    /**
    Set Privacy Guard enabled.

    - Parameters:
        - enable: `True` to enable Privacy Guard, `False` otherwise.
    */
    public static func setEnabled(_ enable: Bool) {
        ODWPrivacyGuard.setEnabled(enable)
    }

    /// Checks if Privacy Guard is enabled, returns `True` if it is, `False` otherwise.
    public static func enabled() -> Bool {
        return ODWPrivacyGuard.enabled()
    }

    /**
    Append fresh Common Data Contexts to the existing instance of Privacy Guard.

    - Parameters:
        - freshCommonDataContext: Fresh Common Data Contexts instance.
    */
    public static func appendCommonDataContext(freshCommonDataContext: ODWCommonDataContext) {
        ODWPrivacyGuard.append(freshCommonDataContext)
    }

    /**
    Add ignored concern to prevent generate of notification signals when this concern is found
    for the given EventName and Field combination.

    - Parameters:
        - eventName: Event name that the ignored concern should apply to.
            - Note: If the ignored concern applies to the Semantic Context Feild, set the Event name to `SemanticContext`.
        - fieldName: Field that the ignored concern should apply to.
        - ignoredConcern: The concern that is expected and should be ignored.
    */
    public static func addIgnoredConcern(eventName: String, fieldName: String, ignoredConcern: ODWDataConcernType) {
        ODWPrivacyGuard.addIgnoredConcern(eventName, with: fieldName, with: ignoredConcern)
    }

    /// Resets the Priavcy Guard instance.
    public static func resetPrivacyGuardInstance() {
        ODWPrivacyGuard.resetPrivacyGuardInstance()
    }
}