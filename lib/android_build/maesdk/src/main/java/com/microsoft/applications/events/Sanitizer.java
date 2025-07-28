//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class Sanitizer  {
    
    /**
     * Initializes the sanitizer with the provided configuration.
     *
     * @param config The configuration object containing logger and event name.
     * @return true if initialization succeeds, false otherwise.
     * @throws IllegalArgumentException if config or any required field is null or invalid.
     */
    public static boolean initialize(SanitizerConfiguration config)  {
        
        // Validate that the configuration object is not null
        if(config == null) {
            throw new IllegalArgumentException("initConfig cannot be null");
        }

        // Ensure the logger instance is provided
        if(config.loggerInstance == null) {
            throw new IllegalArgumentException(("loggerInstance cannot be null in config."));
        }

         // Ensure the notification event name is not null or empty
        if (config.notificationEventName == null || config.notificationEventName.isEmpty()) {
            throw new IllegalArgumentException(("notificationEventName cannot be null in config."));
        }

        return nativeInitialize(config.loggerInstance.getNativeILoggerPtr(), config.notificationEventName);
    }

    /**
     * Checks if the sanitizer is initialized.
     *
     * @return true if initialized, false otherwise.
     */
    public static native boolean isInitialized();

    /**
     * Initializes the sanitizer with the given logger pointer and optional notification event name.
     *
     * @param loggerNativePtr         Native pointer to ILogger.
     * @param notificationEventName  Optional event name for sanitizer notifications.
     * @return true if initialization was successful, false otherwise.
     */
    public static native boolean nativeInitialize(long loggerNativePtr, String notificationEventName);

    /**
     * Uninitializes the sanitizer.
     *
     * @return true if uninitialization was successful, false otherwise.
     */
    public static native boolean uninitialize();

    /**
     * Checks if the sanitizer is currently enabled.
     *
     * @return true if enabled, false otherwise.
     */
    public static native boolean isEnabled();

    /**
     * Enables or disables the sanitizer.
     *
     * @param enabled true to enable, false to disable.
     * @return true if the operation was successful, false otherwise.
     */
    public static native boolean setEnabled(boolean enabled);
}