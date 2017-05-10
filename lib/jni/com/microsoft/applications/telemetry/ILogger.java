// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

/**
 * Interface for logging telemetry events.
 */
public interface ILogger {

    /**
     * Gets an ISemanticContext interface through which to specify semantic
     * context of this logger instance.
     *
     * @return An ISemanticContext instance..
     */
    public ISemanticContext getSemanticContext();

    /**
     * Logs a custom event with specified name and fields to track information
     * such as how a particular feature is used.
     *
     * @param properties Prepared properties of the custom event.
     */
    public void logEvent(EventProperties properties);
}
