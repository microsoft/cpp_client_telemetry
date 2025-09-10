//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * Represents the configuration settings used to initialize a sanitizer.
 */

public class SanitizerConfiguration {

    /**
     * The logger instance used to record privacy concern events.
     * This field is required.
     */
    private final ILogger loggerInstance;

    /**
     * The custom event name used when logging sanitizer concerns.
     * Optional. Defaults to "SanitizerConcerns" if not specified.
     */
    private String notificationEventName = "SanitizerConcerns";

    /**
     * Flag to control whether sanitizer enforcement is enabled.
     * Optional. Defaults to true.
     */
    private boolean enforceSanitization = true;

    /**
     * Constructs a new SanitizerConfiguration with the specified logger.
     *
     * @param logger The ILogger implementation used to log privacy concern events.
     * @throws IllegalArgumentException if the logger is null.
     */
    public SanitizerConfiguration(ILogger logger) {

        if(logger == null) {
            throw new IllegalArgumentException("logger cannot be null");
        }

        this.loggerInstance = logger;
    }

    // Returns the logger instance used to record privacy concern events.
    public ILogger getLogger() {
        return this.loggerInstance;
    }

    // Returns the current event name used for logging sanitizer concerns. Defaults to "SanitizerConcerns" if not specified.
    public String getNotificationEventName() {
        return this.notificationEventName;
    }

    // Sets a custom event name for logging sanitizer concerns.
    // Ignores null or empty strings to preserve the default value.
    public void setEventName(String eventName) {
        if (eventName != null && !eventName.trim().isEmpty()) {
            this.notificationEventName = eventName;
        }
    }

    
    // Returns whether sanitization enforcement is currently enabled.
    public boolean isEnforceSanitization() {
        return enforceSanitization;
    }

   // Sets the flag to enable or disable sanitization enforcement.
    public void setEnforceSanitization(boolean enforceSanitization) {
        this.enforceSanitization = enforceSanitization;
    }
}