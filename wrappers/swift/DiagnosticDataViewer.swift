//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Wrapper class over `ODWDiagnosticDataViewer` representing Diagnostic Data Viewer Hook.
public final class DiagnosticDataViewer {

    /// MARK: Initialization Methods

    /**
    Initializes Data Viewer with a specified machine identifier.

    - Parameters:
        - machineIdentifier: `String` that contains the machine identifier.
    */
    public static func initViewerWithMachineIdentifier(machineIdentifier: String) {
        ODWDiagnosticDataViewer.initializeViewer(withMachineIdentifier: machineIdentifier)
    }

    /// MARK: Behavior Methods

    /**
    Enables Data Viewer.

    - Parameters:
        - endpoint: A `String` that contains endpoint to route events.
        - completionWithResult: Code to execute when enable is completed.
            - Note: This value can be null.

    Execution is dispatched to queue with default priority.
    */
    public static func enableRemoteViewer(endpoint: String, completionWithResult completion: @escaping (Bool) -> Void) {
        ODWDiagnosticDataViewer.enableRemoteViewer(endpoint, completionWithResult: completion)
    }

    /**
    Enables Data Viewer.

    - Parameters:
        - endpoint: A `String` that contains endpoint to route events.
    */
    public static func enableRemoteViewer(endpoint: String) -> Bool {
        return ODWDiagnosticDataViewer.enableRemoteViewer(endpoint)
    }

    /**
    Disables Data Viewer.

    - Parameters:
        - completionWithResult: Code to execute when disable is completed.
            - Note: This value can be null.

    Execution is dispatched to queue with default priority.
    */
    public static func disableViewer(completionWithResult completion: @escaping (Bool) -> Void) {
        ODWDiagnosticDataViewer.disableViewer(completion)
    }

    /// Disables Data Viewer.
    public static func disableViewer() -> Bool {
        return ODWDiagnosticDataViewer.disableViewer()
    }

    /// Returns `True` if Data Viewer is enabled, `False` otherwise.
    public static func viewerEnabled() -> Bool {
        return ODWDiagnosticDataViewer.viewerEnabled()
    }


    /// Returns current endpoint if it is set, empty `String` otherwise.
    public static func currentEndpoint() -> String? {
        return ODWDiagnosticDataViewer.currentEndpoint()
    }

    /**
    Sets callback for OnDisableNotification event.

    - Parameters:
        - run: Code to execute when OnDisableNotification event occurs.
    */
    public static func onDisableNotification(run callback: @escaping () -> Void) {
        ODWDiagnosticDataViewer.register(onDisableNotification: callback)
    }
}