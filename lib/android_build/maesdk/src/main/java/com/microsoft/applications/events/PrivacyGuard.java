//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class PrivacyGuard {

    //Initialize PG
    private static native boolean nativeInitializePrivacyGuard(
            long iLoggerNativePtr,
            String NotificationEventName,
            String SemanticContextEventName,
            String SummaryEventName,
            boolean UseEventFieldPrefix,
            boolean ScanForUrls,
            String domainName,
            String machineName,
            Object[] userNames,
            Object[] userAliases,
            Object[] ipAddresses,
            Object[] languageIdentifiers,
            Object[] machineIds,
            Object[] outOfScopeIdentifiers);

    private static native boolean nativeInitializePrivacyGuardWithoutCommonDataContext(
            long iLoggerNativePtr,
            String NotificationEventName,
            String SemanticContextEventName,
            String SummaryEventName,
            boolean UseEventFieldPrefix,
            boolean ScanForUrls
            );

    /**
     * Initialize Privacy Guard from ILogger
     * @param initConfig Initialization configuration for Privacy Guard
     * @return true if Privacy Guard is successfully initialized, false otherwise.
     * @throws IllegalArgumentException if loggerInstance is null.
     */
    public static boolean initialize(PrivacyGuardInitConfig initConfig)
    {
        if(initConfig == null)
        {
            throw new IllegalArgumentException("initConfig cannot be null");
        }

        if(initConfig.LoggerInstance == null)
        {
            throw new IllegalArgumentException(("loggerInstance cannot be null in initConfig."));
        }

        if(initConfig.DataContext != null)
        {
            return nativeInitializePrivacyGuard(initConfig.LoggerInstance.getNativeILoggerPtr(),
                    initConfig.NotificationEventName,
                    initConfig.SemanticContextNotificationEventName,
                    initConfig.SummaryEventName,
                    initConfig.UseEventFieldPrefix,
                    initConfig.ScanForUrls,
                    initConfig.DataContext.domainName,
                    initConfig.DataContext.machineName,
                    initConfig.DataContext.userNames.toArray(),
                    initConfig.DataContext.userAliases.toArray(),
                    initConfig.DataContext.ipAddresses.toArray(),
                    initConfig.DataContext.languageIdentifiers.toArray(),
                    initConfig.DataContext.machineIds.toArray(),
                    initConfig.DataContext.outOfScopeIdentifiers.toArray());
        } else
        {
            return nativeInitializePrivacyGuardWithoutCommonDataContext(
                    initConfig.LoggerInstance.getNativeILoggerPtr(),
                    initConfig.NotificationEventName,
                    initConfig.SemanticContextNotificationEventName,
                    initConfig.SummaryEventName,
                    initConfig.UseEventFieldPrefix,
                    initConfig.ScanForUrls
            );
        }
    }

    /**
     * Uninitialize the current instance of Privacy Guard
     * This is useful if the app would like to change the logger associated with the instance of Privacy Guard.
     * @return True if Privacy Guard was uninitialized, false if Privacy Guard had not been initialized before.
     */
    public static native boolean uninitialize();

    /**
     * Check if Privacy Guard has been initialized or not.
     * @return `True` if Privacy Guard was initialized, `False` otherwise.
     */
    public static native boolean isInitialized();

    /**
     * Set the Enabled state for Privacy Guard
     * @param isEnabled New Enabled value
     * @return True if Privacy Guard was initialized and the enabled state update was conveyed, False otherwise.
     */
    public static native boolean setEnabled(final boolean isEnabled);

    /**
     * Get the Enabled state for Privacy Guard
     * @return `true` is Privacy Guard is initialized and enabled, `false` otherwise.
     */
    public static native boolean isEnabled();

    private static native boolean nativeAppendCommonDataContext(
            String domainName,
            String machineName,
            Object[] userNames,
            Object[] userAliases,
            Object[] ipAddresses,
            Object[] languageIdentifiers,
            Object[] machineIds,
            Object[] outOfScopeIdentifiers);

    /**
     * Append fresh common data context to current instance of Privacy Guard
     * @param freshDataContext Fresh set of Common Data Context.
     * @return False if either data is null or Privacy Guard has not been initialized yet. True otherwise.
     * @throws IllegalArgumentException if freshDataContext value is null.
     */
    public static boolean appendCommonDataContext(final CommonDataContext freshDataContext)
    {
        if(freshDataContext == null)
        {
            throw new IllegalArgumentException("Passed Common Data Context is null");
        }

        return nativeAppendCommonDataContext(
                freshDataContext.domainName,
                freshDataContext.machineName,
                freshDataContext.userNames.toArray(),
                freshDataContext.userAliases.toArray(),
                freshDataContext.ipAddresses.toArray(),
                freshDataContext.languageIdentifiers.toArray(),
                freshDataContext.machineIds.toArray(),
                freshDataContext.outOfScopeIdentifiers.toArray());
    }

    private static native void nativeAddIgnoredConcern(final String eventName, final String fieldName, final int dataConcern);

    /**
     * Add a known concern to be ignored.
     * @param eventName EventName that might contain a concern. <b>Note:</b> If the ignored concern applies to Semantic Context field, set the Event name to 'SemanticContext'.
     * @param fieldName FieldName that might contain a concern.
     * @param dataConcern Specific DataConcernType you wish to ignore for the given event and field combination.
     */
    public static void addIgnoredConcern(final String eventName, final String fieldName, final DataConcernType dataConcern)
    {
        nativeAddIgnoredConcern(eventName, fieldName, dataConcern.getValue());
    }
}

