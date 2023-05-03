//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

/// Reresents general logging properties.
public class LogConfiguration {
    private var odwLogConfiguration: ODWLogConfiguration

    public init(withConfig config: ODWLogConfiguration) {
        self.odwLogConfiguration = config
    }

    /// Return a copy of the default configuration
    public static func getLogConfigurationCopy() -> LogConfiguration? {
        if let odwLogConfiguration = ODWLogConfiguration.getCopy() {
            return LogConfiguration(withConfig: odwLogConfiguration)
        } else {
            return nil
        }
    }

    /**
    Sets the URI of the event collector.

    - Parameters:
        - collectorUri: String for event collector uri.
    */
    public static func setEventCollectorUri(eventCollectorUri collectorUri: String!) {
        ODWLogConfiguration.setEventCollectorUri(collectorUri)
    }

    /// Returns the URI of the event collector.
    public static func eventCollectorUri() -> String? {
        return ODWLogConfiguration.eventCollectorUri();
    }

    /**
    Sets the RAM queue size limit in bytes.

    - Parameters:
        - sizeInBytes: A long value for memory size limit in bytes.
    */
    public static func setCacheMemorySizeLimitInBytes(withCacheMeomorySizeLimitInBytes sizeInBytes: UInt64) {
        ODWLogConfiguration.setCacheMemorySizeLimitInBytes(sizeInBytes)
    }

    /// Returns the RAM queue size limit in bytes.
    public static func cacheMemorySizeLimitInBytes() -> UInt64 {
        return ODWLogConfiguration.cacheMemorySizeLimitInBytes()
    }

    /**
    Sets the sizse limit of the disk file used to cache events on the client side.

    - Parameters:
        - sizeInBytes: A long value for cache file sizse limit.
    */
    public static func setCacheFileSizeLimitInBytes(withCacheFileSizeLimitInBytes sizeInBytes: UInt64) {
        ODWLogConfiguration.setCacheFileSizeLimitInBytes(sizeInBytes)
    }

    /// Returns the size limit of the disk file used to cache events on the client side.
    public static func cacheFileSizeLimitInBytes() -> UInt64 {
        return ODWLogConfiguration.cacheFileSizeLimitInBytes()
    }

    /**
    Sets max teardown upload time in seconds.

    - Parameters:
        - timeInSecs: An integer that represents time in seconds.
    */
    public static func setMaxTeardownUploadTimeInSec(withMaxTeardownUploadTimeInSec timeInSec: Int32) {
        ODWLogConfiguration.setMaxTeardownUploadTimeInSec(timeInSec)
    }

    /**
    Sets if trace logging to file is enabled.

    - Parameters:
        - trace: True if tracing is enabled.
    */
    public static func setEnableTrace(withEnableTrace trace: Bool) {
        ODWLogConfiguration.setEnableTrace(trace)
    }

    /**
    Sets if console logging from the iOS wrapper is enabled.

    - Parameters:
        - enableLogging: True if logging is enabled.
    */
    public static func setEnableConsoleLogging(withEnableConsoleLogging enableLogging: Bool) {
        ODWLogConfiguration.setEnableConsoleLogging(enableLogging)
    }

    /**
    Sets the internal SDK debugging trace level.

    - Parameters:
        - traceLevel: one of the ACTTraceLevel values.
    */
    public static func setTraceLevel(withTraceLevel traceLevel: Int32) {
        ODWLogConfiguration.setTraceLevel(traceLevel)
    }

    /// Returns true if tracing is enabled
    public static func enableTrace() -> Bool {
        return ODWLogConfiguration.enableTrace()
    }

    /// Returns true if console logging is enabled.
    public static func enableConsoleLogging() -> Bool {
        return ODWLogConfiguration.enableConsoleLogging()
    }

    /**
    Sets if inner exceptions should be surface to Wrapper consumers.

    - Parameters:
        - surfaceObjCExcecptions: True if C++ exceptions should be surfaced.
    */
    public static func setSurfaceCppExceptions(withSurfaceExceptions surfaceExceptions: Bool) {
        ODWLogConfiguration.setSurfaceCppExceptions(surfaceExceptions)
    }

    /// Returns true if inner C++ exceptions are surfaced to Wrapper consumers.
    public static func surfaceCppExceptions() -> Bool {
        return ODWLogConfiguration.surfaceCppExceptions()
    }

    /**
    Sets if session timestamp should be reset after a session ends.

    - Parameters:
        - enableSessionReset: True if session should be reset on session end.
    */
    public static func setEnableSessionReset(withEnableSessionReset sessionReset: Bool) {
        ODWLogConfiguration.setEnableSessionReset(sessionReset)
    }

    /// Returns true if session will be reset on session end.
    public static func enableSessionReset() -> Bool {
        return ODWLogConfiguration.enableSessionReset()
    }

    /**
    Set cache file path.

    - Parameters:
        - cacheFilePath: A string for the cache path.
    */
    public static func setCacheFilePath(withCacheFilePath cacheFilePath: String!) {
        ODWLogConfiguration.setCacheFilePath(cacheFilePath)
    }

    /// Returns the cache file path.
    public static func cacheFilePath() -> String? {
        return ODWLogConfiguration.cacheFilePath()
    }

    /**
    Controls if DB will be checkpointed when flushing.

    - Parameters:
        - enableDBCheckpointOnFlush: True if DB should be checkpointed when flushing.
    */
    public static func setEnableDBCheckpointOnFlush(withEnableDBCheckpointOnFlush enableDBCheckpointOnFlush: Bool) {
        ODWLogConfiguration.setEnableDbCheckpointOnFlush(enableDBCheckpointOnFlush)
    }

    /// Returns true if DB will checkpointed when flushing.
    public static func enableDBCheckpointOnFlush() -> Bool {
        return ODWLogConfiguration.enableDbCheckpointOnFlush()
    }

    /**
    Sets a config key to a string value for the copied config

    - Parameters:
        - key: A key
        - value: A value
    */
    public func set(withKey key: String!, withValue value: String!) {
        self.odwLogConfiguration.set(key, withValue: value)
    }

    /// Returns the value for the given key.
    public func valueForKey(key: String!) -> String? {
        return self.odwLogConfiguration.value(forKey: key)
    }

    /**
    Sets the host value

    - Parameters:
        - host: A host.
    */
    public func setHost(host: String!) {
        self.odwLogConfiguration.setHost(host)
    }

    /// Returns the host value.
    public func host() -> String? {
        return self.odwLogConfiguration.host()
    }
}