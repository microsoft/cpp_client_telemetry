//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Wrapper over ODWLogManager which manages the telemetry logging system.
public class LogManager {

    /**
    Initializes the telemetry logging system with the specified tenant token and custom configuration.

    - Parameters:
        - tenantToken: A `String` that contains the tenant token.
        - config: A `Dictionary` that contains a custom configuration.

    - Returns:
        - A `Logger` instance.
    */
    public static func initForTenant(tenantToken: String!, config: [AnyHashable: Any]? = nil) -> Logger? {
        if let odwLogger = ODWLogManager.initForTenant(tenantToken, withConfig: config) {
            return Logger(logger: odwLogger)
        } else {
            return nil
        }
    }

    /**
    Retrieves a new instance of `Logger` for logging telemetry events. Initializes the telemetry logging system,
    if not already initialized, with the default configuration using the specified tenant token and source.

    - Parameters:
        - tenantToken: A `String` that contains the tenant token.
        - withSource: A `String` that contains the name of the source of events.

    - Returns:
        - A `Logger` instance that points to the logger for the specified tenant token and source.
    */
    public static func loggerWithTenant(tenantToken: String!, withSource source: String! = "") -> Logger? {
        if let odwLogger = ODWLogManager.logger(withTenant: tenantToken, source: source) {
            return Logger(logger: odwLogger)
        } else {
            return nil
        }
    }

    /**
    Retrieves a new instance of `Logger` for logging telemetry events.
    This method expects that the logging system has already been initialized.

    - Parameters:
        - tenantToken: A `String` that contains the tenant token.
        - withSource: A `String` that contains the name of the source of events.
        - config: A `Dictionary` that contains a custom configuration.

    - Returns:
        - A `Logger` instance that points to the logger for the specified tenant token and source.
    */
    public static func loggerWithTenant(tenantToken: String!, withSource source: String!, withConfig config: ODWLogConfiguration!) -> Logger?{
        if let odwLogger = ODWLogManager.logger(withTenant: tenantToken, source: source, withConfig: config) {
            return Logger(logger: odwLogger)
        } else {
            return nil
        }
    }

    /**
    Retrieves a new instance of `Logger` for logging telemetry events.

    - Note:
        It requires to previously call `loggerWithTenant` method.

    - Parameters:
        - source: A `String` that contains the name of the source of events sent by this logger instance.

    - Return:
        - A `Logger` instance that points to the logger for source.
    */
    public static func loggerForSource(source: String!) -> Logger? {
        if let odwLogger = ODWLogManager.logger(forSource: source) {
            return Logger(logger: odwLogger)
        } else {
            return nil
        }
    }

    /// Attempts to send any pending telemetry events that are currently cached either in memory, or on disk.
    /// Use this method if your event can't wait for automatic timed upload.
    public static func uploadNow() {
        ODWLogManager.uploadNow()
    }

    /// Flushes pending telemetry events from memory to disk (to reduce possible data loss) and returns the flush operation's status.
    public static func flush() throws -> ODWStatus {
        do {
            return ODWLogManager.flush()
        } catch let exception as NSException {
            if (LogConfiguration.surfaceCppExceptions()) {
                try Logger.raiseException(withMessage: exception.reason)
            }

            return ODWStatus.efail
        }
    }

    /// Flushes pending telemetry events from memory to disk, tears-down the telemetry logging system, and returns the flush operation's status.
    public static func flushAndTeardown() throws -> ODWStatus {
        do {
            return ODWLogManager.flushAndTeardown()
        } catch let exception as NSException {
            if (LogConfiguration.surfaceCppExceptions()) {
                try Logger.raiseException(withMessage: exception.reason)
            }

            return ODWStatus.efail
        }
    }

    /**
    Sets the transmit profile for event transmission.

    - Details:
        A transmit profile is a collection of hardware and system settings (like network connectivity, power state etc.)
                    that determines how efficiently telemetry events are transmitted.

    - Parameters:
        - profile: The transmit profile to set, as one of the `ODWTransmissionProfile` values.
    */
    public static func setTransmissionProfile(profile: ODWTransmissionProfile) {
        ODWLogManager.setTransmissionProfile(profile)
    }

    /// Pauses transmission of telemetry events to the data collector.
    /// While paused, events continue to be queued on the client side, cached either in memory or on disk.
    public static func pauseTransmission() throws {
        do {
            ODWLogManager.pauseTransmission()
        } catch let exception as NSException {
            if (LogConfiguration.surfaceCppExceptions()) {
                try Logger.raiseException(withMessage: exception.reason)
            }
        }
    }

    /// Resumes the transmission of the telemetry events to the data collector.
    public static func resumeTransmission() {
        ODWLogManager.resumeTransmission()
    }

    /// Resets the transmit profiles to contain only default profiles.
    public static func resetTransmitProfiles() {
        ODWLogManager.resetTransmitProfiles()
    }

    /**
    Adds (or overrides) a property of the context for the telemetry logging system and tags it with its PII kind.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withValue: A `String` that contains the value of the property.
    */
    public static func setContextWithName(name: String!, withValue value:String!) {
        ODWLogManager.setContextWithName(name, stringValue:value)
    }

    /**
    Adds (or overrides) a property of the context for the telemetry logging system and tags it with its PII kind.

    - Parameters:
        - name: A `String` that contains the name of the property.
        - withValue: A `String` that contains the value of the property.
        - withPiiKind: The kind of Personal Identifiable Information (PII), as one of the `ODWPiiKind` enum values.
    */
    public static func setContextWithName(name: String!, withValue value:String!, withPiiKind piiKind: ODWPiiKind) {
        ODWLogManager.setContextWithName(name, stringValue:value, piiKind: piiKind)
    }

    /// Host apps should call this method when the app receives applicationWillTerminate notification.
    /// Calling this API ensure safe termination of the logging libraray on exit.
    public static func applicationWillTerminate() {
        ODWLogManager.applicationWillTerminate()
    }
}
