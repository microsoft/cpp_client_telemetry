//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Wrapper class around `ODWDiagnosticDataViewer` which represents Diagnostic Data Viewer Hook.
public class DiagnosticDataViewer {
    private var odwDiagnosticDataViewer: ODWDiagnosticDataViewer

    // MARK: Initialization methods

    /// Constructs object with machineIdentifier taken as `String`.
    public func init(withMachineIdentifier: String) {
        self.odwDiagnosticDataViewer = ODWDiagnosticDataViewer(machineIdentifier: withMachineIdentifier)
    }

    // MARK: Behavior methods

    public func enableRemoteViewer(endpoint: String, completionWithResult: )
}