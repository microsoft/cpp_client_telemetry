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
     * The logger instance used to record sanitizer concern events.
     * This field is required.
     */
    private final ILogger loggerInstance;
   
    /**
     * The custom event name used when logging sanitizer concerns.
     * Optional. Defaults to "SanitizerConcerns" if not specified.
     */
    private String notificationEventName = "SanitizerConcerns";

    /**
     * Flag to control whether warnings should be turned off.
     * Optional. Defaults to true.
     */
    private boolean warningsOff = true;

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

    // Getters and Setters
    public ILogger getLogger() {
        return this.loggerInstance;
    }

    public String getNotificationEventName() {
        return this.notificationEventName;
    }

    public void setEventName(String eventName) {
        if (eventName != null && !eventName.trim().isEmpty()) {
            this.notificationEventName = eventName;
        }
    }

    public boolean isWarningsOff() {
        return warningsOff;
    }

    public void setWarningsOff(boolean warningsOff) {
        this.warningsOff = warningsOff;
    }
}