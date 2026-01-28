//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class Sanitizer  {    
    /**
     * Initializes the sanitizer with the given logger pointer and optional notification event name.
     *
     * @param loggerNativePtr         Native pointer to ILogger.
     * @param notificationEventName   Optional event name for sanitizer notifications.
     * @param enforceSanitization     Flag to control whether sanitization is enforced.
     * @param urlDomains             Array of URL domains to allow.
     * @param emailDomains           Array of email domains to allow.
     * @param analyzerOptions        Analyzer options flags (bitwise OR of values):
     *                               0 = None (default - no special analyzer behaviors). SitePathLoose is the default behavior.
     *                               1 = SitePathStrict (enables strict site path analysis)
     *                               2 = SitePathLoose (enables loose site path analysis)
     *                               Multiple flags can be combined with bitwise OR (e.g., 1 | 2 = 3)
     * @param sendConcernLimit       Maximum number of concerns to send.
     * @param insertWarningAtProblemLocation Insert warnings at problem location (true) or prepend (false).
     * @return true if initialization was successful, false otherwise.
     */
    private static native boolean nativeInitialize(long loggerNativePtr,
        String notificationEventName,
        boolean enforceSanitization,
        String[] urlDomains,
        String[] emailDomains,
        int analyzerOptions,
        int sendConcernLimit,
        boolean insertWarningAtProblemLocation);    
    /**
     * Initializes the sanitizer with the provided configuration.
     *
     * @param config The configuration settings used to initialize a sanitizer.
     * @param urlDomains Array of URL domains to allow (can be null for empty list).
     * @param emailDomains Array of email domains to allow (can be null for empty list).
     * @param analyzerOptions Analyzer options flags (bitwise OR of values):
     *                        0 = None (default - no special analyzer behaviors). SitePathLoose is the default behavior.
     *                        1 = SitePathStrict (enables strict site path analysis)
     *                        2 = SitePathLoose (enables loose site path analysis)
     *                        Multiple flags can be combined with bitwise OR (e.g., 1 | 2 = 3)
     * @param sendConcernLimit Maximum number of concerns to send. 0 = no concerns sent, 65536+ = all concerns sent.
     * @return true if initialization succeeds, false otherwise.
     * @throws IllegalArgumentException if config or any required field is null or invalid.
     */
    public static boolean initialize(SanitizerConfiguration config, String[] urlDomains, String[] emailDomains, int analyzerOptions, int sendConcernLimit)  {
        
        // Validate that the configuration object is not null
        if(config == null) {
            throw new IllegalArgumentException("initConfig cannot be null");
        }

        // Ensure the logger instance is provided
        if(config.getLogger() == null) {
            throw new IllegalArgumentException(("loggerInstance cannot be null in config."));
        }

         // Ensure the notification event name is not null or empty
        if (config.getNotificationEventName() == null || config.getNotificationEventName().isEmpty()) {
            throw new IllegalArgumentException(("notificationEventName cannot be null in config."));
        }

        return nativeInitialize(
            config.getLogger().getNativeILoggerPtr(),
            config.getNotificationEventName(),
            config.isEnforceSanitization(),
            urlDomains,
            emailDomains,
            analyzerOptions,
            sendConcernLimit,
            config.isInsertWarningAtProblemLocation());
    }

    /**
     * Checks if the sanitizer is initialized.
     *
     * @return true if initialized, false otherwise.
     */
    public static native boolean isInitialized();

    
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