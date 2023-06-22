//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

import ObjCModule

/// Class wrapping `ODWLogConfiguration` ObjC class object, representing configuration related to events.
public final class LogConfiguration {
    private var _odwLogConfiguration: ODWLogConfiguration
    var odwLogConfiguration:ODWLogConfiguration {
        get {
            odwLogConfiguration
        }
    }

    /**
    Constructs `self` with provided config as `ODWLogConfiguration` object.

    - Parameters:
        - config: `ODWLogConfiguration` object which would be wrapped to.
    */
    init(config: ODWLogConfiguration) {
        self._odwLogConfiguration = config
    }

    /// Return a copy of the default configuration
    public static func getLogConfigurationCopy() -> LogConfiguration? {
        if let _odwLogConfiguration = ODWLogConfiguration.getCopy() {
            return LogConfiguration(config: _odwLogConfiguration)
        } else {
            return nil
        }
    }

    /**
    Sets the "URI" of the event collector.

    - Parameters:
        - collectorUri: `String` for event collector uri.
    */
    public static func setEventCollectorUri(_ collectorUri: String) {
        ODWLogConfiguration.setEventCollectorUri(collectorUri)
    }

    /// Returns a `String` containing "URI" of the event collector.
    public static func eventCollectorUri() -> String? {
        return ODWLogConfiguration.eventCollectorUri();
    }

    /**
    Sets the RAM queue size limit in bytes.

    - Parameters:
        - sizeInBytes: A `Long` value for memory size limit in bytes.
    */
    public static func setCacheMemorySizeLimitInBytes(_ sizeInBytes: UInt64) {
        ODWLogConfiguration.setCacheMemorySizeLimitInBytes(sizeInBytes)
    }

    /// Returns the RAM queue size as `Long` value having limit in bytes.
    public static func cacheMemorySizeLimitInBytes() -> UInt64 {
        return ODWLogConfiguration.cacheMemorySizeLimitInBytes()
    }

    /**
    Sets the size limit of the disk file used to cache events on the client side.

    - Parameters:
        - sizeInBytes: A `Long` value for cache file size limit.
    */
    public static func setCacheFileSizeLimitInBytes(_ sizeInBytes: UInt64) {
        ODWLogConfiguration.setCacheFileSizeLimitInBytes(sizeInBytes)
    }

    /// Returns the size limit of the disk file as a `Long` used to cache events on the client side.
    public static func cacheFileSizeLimitInBytes() -> UInt64 {
        return ODWLogConfiguration.cacheFileSizeLimitInBytes()
    }

    /**
    Sets max teardown upload time in seconds.

    - Parameters:
        - timeInSecs: An `Integer` that represents time in seconds.
    */
    public static func setMaxTeardownUploadTimeInSec(_ timeInSecs: Int32) {
        ODWLogConfiguration.setMaxTeardownUploadTimeInSec(timeInSecs)
    }

    /**
    Sets if trace logging to file is enabled.

    - Parameters:
        - trace: `True` if tracing should be enabled, `False` otherwise.
    */
    public static func setEnableTrace(_ trace: Bool) {
        ODWLogConfiguration.setEnableTrace(trace)
    }

    /**
    Sets if console logging from the iOS wrapper is enabled.

    - Parameters:
        - enableLogging: `True` if logging is enabled, `False` otherwise.
    */
    public static func setEnableConsoleLogging(_ enableLogging: Bool) {
        ODWLogConfiguration.setEnableConsoleLogging(enableLogging)
    }

    /**
    Sets the internal SDK debugging trace level.

    - Parameters:
        - traceLevel: one of the `ACTTraceLevel` values.
    */
    public static func setTraceLevel(_ traceLevel: Int32) {
        ODWLogConfiguration.setTraceLevel(traceLevel)
    }

    /// Returns `True` if tracing is enabled, `False` otherwise.
    public static func enableTrace() -> Bool {
        return ODWLogConfiguration.enableTrace()
    }

    /// Returns `True` if console logging is enabled, `False` otherwise.
    public static func enableConsoleLogging() -> Bool {
        return ODWLogConfiguration.enableConsoleLogging()
    }

    /**
    Sets if inner exceptions should be surface to Wrapper consumers.

    - Parameters:
        - surfaceObjCExcecptions: `True` if C++ exceptions should be surfaced, `False` otherwise.
    */
    public static func setSurfaceCppExceptions(_ surfaceExceptions: Bool) {
        ODWLogConfiguration.setSurfaceCppExceptions(surfaceExceptions)
    }

    /// Returns `True` if inner C++ exceptions are surfaced to Wrapper consumers, `False` otherwise.
    public static func surfaceCppExceptions() -> Bool {
        return ODWLogConfiguration.surfaceCppExceptions()
    }

    /**
    Sets if session timestamp should be reset after a session ends.

    - Parameters:
        - enableSessionReset: `True` if session should be reset on session end, `False` otherwise.
    */
    public static func setEnableSessionReset(_ sessionReset: Bool) {
        ODWLogConfiguration.setEnableSessionReset(sessionReset)
    }

    /// Returns `True` if session will be reset on session end, `False` otherwise.
    public static func enableSessionReset() -> Bool {
        return ODWLogConfiguration.enableSessionReset()
    }

    /**
    Set cache file path.

    - Parameters:
        - cacheFilePath: A `String` for the cache path.
    */
    public static func setCacheFilePath(_ cacheFilePath: String) {
        ODWLogConfiguration.setCacheFilePath(cacheFilePath)
    }

    /// Returns the cache file path as a `String`.
    public static func cacheFilePath() -> String? {
        return ODWLogConfiguration.cacheFilePath()
    }

    /**
    Controls if DB will be checkpointed when flushing.

    - Parameters:
        - enableDBCheckpointOnFlush: `True` if DB should be checkpointed when flushing.
    */
    public static func setEnableDBCheckpointOnFlush(_ enableDBCheckpointOnFlush: Bool) {
        ODWLogConfiguration.setEnableDbCheckpointOnFlush(enableDBCheckpointOnFlush)
    }

    /// Returns `True` if DB will checkpointed when flushing, `False` otherwise.
    public static func enableDBCheckpointOnFlush() -> Bool {
        return ODWLogConfiguration.enableDbCheckpointOnFlush()
    }

    /**
    Sets a config key to a string value for the copied config.

    - Parameters:
        - key: A key as a `String`.
        - value: A value as a `String`.
    */
    public func set(_ key: String, havingValue value: String) {
        _odwLogConfiguration.set(key, withValue: value)
    }

    /// Returns the value as `String` for the given `String` key.
    public func value(forKey key: String) -> String? {
        return _odwLogConfiguration.value(forKey: key)
    }

    /**
    Sets the host value.

    - Parameters:
        - host: A host value as `String`.
    */
    public func setHost(_ host: String) {
        _odwLogConfiguration.setHost(host)
    }

    /// Returns the host value as `String`.
    public func host() -> String? {
        return _odwLogConfiguration.host()
    }
}
