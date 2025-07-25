//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

/**
 * Represents the configuration settings used to initialize a sanitizer instance.
 */

public class SanitizerConfiguration {

    /**
     * The logger instance used to record privacy concern events.
     * This field is required.
     */
    public final ILogger loggerInstance;
   
    /**
     * The custom event name used when logging sanitizer concerns.
     * Optional. Defaults to "SanitizerConcerns" if not specified.
     */
    public String notificationEventName = "SanitizerConcerns";

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
}