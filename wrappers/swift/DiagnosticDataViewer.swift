//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Wrapper class over `ODWDiagnosticDataViewer` representing Diagnostic Data Viewer Hook.
public class DiagnosticDataViewer {
    /**
    Initializes Data Viewer with a specified machine identifier.

    - Parameters:
        - machineIdentifier: `String` that contains the machine identifier.
    */
    public static func initWithMachineIdentifier(_ machineIdentifier: String) {
        ODWDiagnosticDataViewer.initializeWithMachineIdentifier(machineIdentifier)
    }
}