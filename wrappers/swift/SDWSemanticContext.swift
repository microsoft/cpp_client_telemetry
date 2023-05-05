//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Wrapper over `ODWSemanticContext` class that manages the inclusion of semantic context values on logged events.
public class SemanticContext {
    private var odwSemanticContext: ODWSemanticContext

    /// Contructs the object with ODWSemanticContext object to be wrapped around.
    public init(semanticContext: ODWSemanticContext) {
        self.odwSemanticContext = semanticContext
    }

    /**
    Specifies an app id to be included with every event.

    - Parameters:
        - appID: A `String` that contains the application identifier.
    */
    public func setAppID(_ appID: String) {
        self.odwSemanticContext.setAppId(appID)
    }

    /**
    Set the application version context information of the telemetery event.

    - Parameters:
        - appVersion: Version of the application, retrieved programmatically where possible and is app/platform specific.
    */
    public func setAppVersion(_ appVersion: String!) {
        self.odwSemanticContext.setAppVersion(appVersion)
    }

    /**
    Set the application language context information of the telemetry event.

    - Parameters:
        - appLanguage: A `String` that contains the spoken/written language used in the application.
    */
    public func setAppLanguage(_ appLanguage: String!) {
        self.odwSemanticContext.setAppLanguage(appLanguage)
    }

    /**
    Specifies a unique user id to be included with every event.

    - Parameters:
        - userID: A `String` that contains the unique user identifier.
        - withPiiKind: A PIIKind of the userID. Set it to PiiKind_None t odenote it as non-PII.
            - Note: Default value is `ODWPiiKind.identity`.
    */
    public func setUserID(_ userID: String!, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.identity) {
        self.odwSemanticContext.setUserId(userID)
    }

    /**
    Set the device identifier context information of the telemetry event.

    - Parameters:
        - deviceID: A unique device identifier, retrieved programmatically where possible and is app/platform speicific.
    */
    public func setDeviceID(_ deviceID: String!) {
        self.odwSemanticContext.setDeviceId(deviceID)
    }

    public func setUserTimeZone(_ userTimeZone: String!) {
        
    }
}
