//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import ObjCModule

/// Wrapper to `ODWSanitizer` representing the Sanitizer.
public final class Sanitizer {

    /**
    Set Sanitizer enabled.

    - Parameters:
        - enable: `True` to enable the Sanitizer, `False` otherwise.
    */
    public static func setEnabled(_ enable: Bool) {
        ODWSanitizer.setEnabled(enable)
    }

    /// Checks if the Sanitizer is enabled, returns `True` if it is, `False` otherwise.
    public static func enabled() -> Bool {
        return ODWSanitizer.enabled()
    }

    /// Resets the Sanitizer instance.
    public static func resetSanitizerInstance() {
        ODWSanitizer.resetSanitizerInstance()
    }
}