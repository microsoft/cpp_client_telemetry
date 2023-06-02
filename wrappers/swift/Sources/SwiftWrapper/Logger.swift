//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Wrapper class around ObjC Logger class `ODWLogger` used to events.
public final class Logger {
    private var odwLogger: ODWLogger

    /// Constructs `Logger` with `ODWLogger` object which would be wrapped around.
    public init(logger: ODWLogger) {
        self.odwLogger = logger
    }

    // MARK: Basic LogEvent methods

    /**
    Logs a custom event with a specified name.

    - Parameters:
        - name: A `String` that contains the name of the event.
    */
    public func logEvent(name: String) {
        odwLogger.logEvent(withName: name)
    }

    /**
    Logs a custom event with a specified set of properties.

    - Parameters:
        - eventProperties: The custome event properties, as `EventProperties`.
    */
    public func logEvent(properties: EventProperties) {
        odwLogger.logEvent(with: properties.getODWEventPropertiesObject())
    }

    // MARK: Semantic LogEvent methods

    /**
    Logs a failure event (such as an app exception), taking a signature, failure details, and event properties.

    - Parameters:
        - signature: A `String` that identifies the bucket of the failure.
        - eventDetail: A `String` that contains a description of the failure.
        - eventProperties: Properties of the failure event, encapsulated within an `EventProperties`.
    */
    public func logFailureWithSignature(_ signature: String, eventDetail detail: String, eventProperties properties: EventProperties) {
        odwLogger.logFailure(withSignature: signature, detail: detail, eventproperties: properties.getODWEventPropertiesObject())
    }

    /**
    Logs a failure with event (such as an app exception) with all the details.

    - Parameters:
        - signature: A `String` that identifies the bucket of the failure.
        - eventDetail: A `String` that contains a description of the failure.
        - eventCategory: A `String` that represents category of the event.
        - eventID: Event ID as a `String`
        - eventProperties: Properties of the failure event, encapsulated within an `EventProperties`.
    */
    public func logFailureWithSignature(_ signature: String,
                                        eventDetail detail: String,
                                        eventCategory category: String,
                                        eventID id: String,
                                        eventProperties properties: EventProperties) {
        odwLogger.logFailure(withSignature: signature, detail: detail, category: category, id: id, eventProperties: properties.getODWEventPropertiesObject())
    }

    /**
    Logs a page action event.

    - Parameters:
        - identifier: `String` that contains unquely identifiable page view.
        - pageName: Page name as a `String`.
        - eventProperties: Properties of the failure event, encapsulated within an `EventProperties`.
    */
    public func logPageViewWithID(_ identifier: String, pageName: String, eventProperties properties: EventProperties) {
        odwLogger.logPageView(withId: identifier, pageName: pageName, eventProperties: properties.getODWEventPropertiesObject())
    }

    /**
    Logs a page action event.

    - Parameters:
        - identifier: `String` that contains unquely identifiable page view.
        - pageName: Page name as a `String`.
        - eventCategory: A `String` that contains name of the category this event belongs to.
        - uri: `String` that contains "URI" of this page.
        - referrerURI: `String` that contains "URI" that refers to this page.
        - eventProperties: Properties of the failure event, encapsulated within an `EventProperties`.
    */
    public func logPageViewWithID(_ identifier: String,
                                    pageName: String,
                                    eventCategory category: String,
                                    uri: String,
                                    referrerURI: String,
                                    eventProperties properties: EventProperties) {
        odwLogger.logPageView(withId: identifier, pageName: pageName, category: category, uri: uri, referrerUri: referrerURI, eventProperties: properties.getODWEventPropertiesObject())
    }

    /**
    Logs a diagnostic trace event to help you troubleshoot problems.

    - Parameters:
        - traceLevel: Level of trace as one of `ODWTraceLevel` enum values.
        - traceMessage: `String` that contains description of the trace.
        - eventProperties: Properties of the failure event, encapsulated within an `EventProperties`.
    */
    public func logTraceWithTraceLevel(_ traceLevel: ODWTraceLevel, traceMessage message: String, eventProperties properties: EventProperties) {
        odwLogger.logTrace(with: traceLevel, message: message, eventProperties: properties.getODWEventPropertiesObject())
    }

    /**
    Logs the session state.

    - Note: Logging *session start* while a session is already exists produces a no-op.
            Similarly, logging *session end* while a session doesn't exist produces a no-op.

    - Parameters:
        - state: The session's state as one of the `ODWSessionState` enum values.
        - eventProperties: Properties of the failure event, encapsulated within an `EventProperties`.
    */
    public func logSessionWithState(_ state: ODWSessionState, eventProperties properties: EventProperties) {
        odwLogger.logSession(with: state, eventProperties: properties.getODWEventPropertiesObject())
    }

    /**
    Initializes and gets an instance of Privacy Guard.
    */
    public func apply(config initConfigObject: PrivacyGuardInitConfig) {
        odwLogger.initializePrivacyGuard(with: initConfigObject.getODWPrivacyGuardInitConfig())
    }

    // MARK: Set Context methods

    /**
    Adds (or overrides) a property of context for the telemetry logging system.
    Context information applies to event generated by the CPP Logger instance, unless it is overwritten on a particular event property.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withStringValue: A `String` that contains the property of the value.
        - withPiiKind: The kind of "Personal Identifiable Information (PII)", as one of the `ODWPiiKind` enum values.
    */
    public func setContextWithName(_ name: String, withStringValue value: String, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.none) {
        odwLogger.setContextWithName(name, stringValue: value, piiKind: piiKind)
    }

    /**
    Adds (or overrides) a property of context for the telemetry logging system.
    Context information applies to event generated by the CPP Logger instance, unless it is overwritten on a particular event property.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withBoolValue: A `Bool` that contains the property of the value.
        - withPiiKind: The kind of "Personal Identifiable Information (PII)", as one of the `ODWPiiKind` enum values.
    */
    public func setContextWithName(_ name: String, withBoolValue value: Bool, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.none) {
        odwLogger.setContextWithName(name, boolValue: value, piiKind: piiKind)
    }

    /**
    Adds (or overrides) a property of context for the telemetry logging system.
    Context information applies to event generated by the CPP Logger instance, unless it is overwritten on a particular event property.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withDateValue: A `Date` that contains the property of the value.
        - withPiiKind: The kind of "Personal Identifiable Information (PII)", as one of the `ODWPiiKind` enum values.
    */
    public func setContextWithName(_ name: String, withDateValue value: Date, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.none) {
        odwLogger.setContextWithName(name, dateValue: value, piiKind: piiKind)
    }

    /**
    Adds (or overrides) a property of context for the telemetry logging system.
    Context information applies to event generated by the CPP Logger instance, unless it is overwritten on a particular event property.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withDoubleValue: A `Double` that contains the property of the value.
        - withPiiKind: The kind of "Personal Identifiable Information (PII)", as one of the `ODWPiiKind` enum values.
    */
    public func setContextWithName(_ name: String, withDoubleValue value: Double, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.none) {
        odwLogger.setContextWithName(name, doubleValue: value, piiKind: piiKind)
    }

    /**
    Adds (or overrides) a property of context for the telemetry logging system.
    Context information applies to event generated by the CPP Logger instance, unless it is overwritten on a particular event property.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withInt64Value: A `Int64` that contains the property of the value.
        - withPiiKind: The kind of "Personal Identifiable Information (PII)", as one of the `ODWPiiKind` enum values.
    */
    public func setContextWithName(_ name: String, withInt64Value value: Int64, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.none) {
        odwLogger.setContextWithName(name, int64Value: value, piiKind: piiKind)
    }

    /**
    Adds (or overrides) a property of context for the telemetry logging system.
    Context information applies to event generated by the CPP Logger instance, unless it is overwritten on a particular event property.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withInt32Value: A `Int32` that contains the property of the value.
        - withPiiKind: The kind of "Personal Identifiable Information (PII)", as one of the `ODWPiiKind` enum values.
    */
    public func setContextWithName(_ name: String, withInt32Value value: Int32, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.none) {
        odwLogger.setContextWithName(name, int32Value: value, piiKind: piiKind)
    }

    /**
    Adds (or overrides) a property of context for the telemetry logging system.
    Context information applies to event generated by the CPP Logger instance, unless it is overwritten on a particular event property.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withUUIDValue: A `UUID` that contains the property of the value.
        - withPiiKind: The kind of "Personal Identifiable Information (PII)", as one of the `ODWPiiKind` enum values.
    */
    public func setContextWithName(_ name: String, withUUIDValue value: UUID, withPiiKind piiKind: ODWPiiKind = ODWPiiKind.none) {
        odwLogger.setContextWithName(name, uuidValue: value, piiKind: piiKind)
    }
}
