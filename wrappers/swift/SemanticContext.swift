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
        odwSemanticContext.setAppId(appID)
    }

    /**
    Set the application version context information of the telemetery event.

    - Parameters:
        - appVersion: Version of the application, retrieved programmatically where possible and is app/platform specific.
    */
    public func setAppVersion(_ appVersion: String) {
        odwSemanticContext.setAppVersion(appVersion)
    }

    /**
    Set the application language context information of the telemetry event.

    - Parameters:
        - appLanguage: A `String` that contains the spoken/written language used in the application.
    */
    public func setAppLanguage(_ appLanguage: String) {
        odwSemanticContext.setAppLanguage(appLanguage)
    }

    /**
    Specifies a unique user id to be included with every event.

    - Parameters:
        - userID: A `String` that contains the unique user identifier.
        - withPiiKind: A PIIKind of the userID. Set it to PiiKind_None t odenote it as non-PII.
            - Note: Default value is `ODWPiiKind.identity`.
    */
    public func setUserID(_ userID: String, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.identity) {
        odwSemanticContext.setUserId(userID)
    }

    /**
    Set the device identifier context information of the telemetry event.

    - Parameters:
        - deviceID: A unique device identifier, retrieved programmatically where possible and is app/platform speicific.
    */
    public func setDeviceID(_ deviceID: String) {
        odwSemanticContext.setDeviceId(deviceID)
    }

    /**
    Set the user time zone context information of telemetry event.

    - Parameters:
        - userTimeZone: User's time zone relative to UTC, in ISO 8601 time zone format.
    */
    public func setUserTimeZone(_ userTimeZone: String) {
        odwSemanticContext.setUserTimeZone(userTimeZone)
    }

    /**
    Specifies a unique user advertising id to be included with every event.

    - Parameters:
        - userAdvertisingID: A `String` that contains the unique user advertising identifier.
    */
    public func setUserAdvertisingID(_ userAdvertisingID: String) {
        odwSemanticContext.setUserAdvertisingId(userAdvertixingID)
    }

    /**
    Sets the experimentation IDs for determining the deployment configuration.

    - Parameters:
        - experimentIDs: A `String` that contains the experimentation IDs.
    */
    public func setAppExperimentIDs(_ experimentIDs: String) {
        odwSemanticContext.setAppExperimentIds(experimentIDs)
    }

    /**
    Sets the experimentation IDs for the specified telemetry event.

    - Parameters:
        - experimentIDs: A `String` that contains the experimentation IDs.
        - eventName: A `String` that contains the name of the event.
    */
    public func setAppExperimentIDs(_ experimentIDs: String, forEvent eventName: String) {
        odwSemanticContext.setAppExperimentIds(experimentIDs, forEvent: eventName)
    }

    /**
    Sets the experiment tag (experiment configuration) context information for telemetry events.

    - Note: This method removes any previously stored experiment IDs that were set using `setAppExperimentEtag`.

    - Parameters:
        - eTag: A `String` that contains the ETag which is a hash of the set of experiments.
    */
    public func setAppExperimentETag(_ eTag: String) {
        odwSemanticContext.setAppExperimentETag(eTag)
    }

    /**
    Sets the impression ID (an identifier of the currently running flights) for an experiment.

    - Parameters:
        - impressionID: A `String` that contains the impression ID for the currently active configuration.

    - Note: Calling this method removes the previously stored experimentation IDs and flight IDs.
    */
    public func setAppExperimentImpressionID(_ impressionID: String) {
        odwSemanticContext.setAppExperimentImpressionId(impressionID)
    }
}
