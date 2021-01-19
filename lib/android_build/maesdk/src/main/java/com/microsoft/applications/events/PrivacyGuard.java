//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class PrivacyGuard {

    //Initialize PG
    private static native boolean nativeInitializePrivacyGuard(
            long iLoggerNativePtr,
            String domainName,
            String machineName,
            String userName,
            String userAlias,
            Object[] ipAddresses,
            Object[] languageIdentifiers,
            Object[] machineIds,
            Object[] outOfScopeIdentifiers);

    private static native boolean nativeInitializePrivacyGuardWithoutCommonDataContext(long iLoggerNativePtr);

    /**
     * Initialize Privacy Guard from ILogger
     * @param loggerInstance ILogger instance that will be used to send data concerns to
     * @param dataContext Common Data Context to initialize Privacy Guard with.
     * @return true if Privacy Guard is successfully initialized, false otherwise. Try UnInit before re-init.
     * @throws IllegalArgumentException if loggerInstance is null.
     */
    public static boolean initializePrivacyGuard(ILogger loggerInstance, final CommonDataContext dataContext)
    {
        if(loggerInstance == null)
        {
            throw new IllegalArgumentException(("loggerInstance cannot be null."));
        }

        if(dataContext != null)
        {
            return nativeInitializePrivacyGuard(loggerInstance.getNativeILoggerPtr(),
                    dataContext.domainName,
                    dataContext.machineName,
                    dataContext.userName,
                    dataContext.userAlias,
                    dataContext.ipAddresses.toArray(),
                    dataContext.languageIdentifiers.toArray(),
                    dataContext.machineIds.toArray(),
                    dataContext.outOfScopeIdentifiers.toArray());
        } else
        {
            return nativeInitializePrivacyGuardWithoutCommonDataContext(loggerInstance.getNativeILoggerPtr());
        }
    }

    /**
     * Uninitialize the current instance of Privacy Guard
     * This is useful if the app would like to change the logger associated with the instance of Privacy Guard.
     * @return True if Privacy Guard was uninitialized, false if Privacy Guard had not been initialized before.
     */
    public static native boolean uninitializePrivacyGuard();

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
            String userName,
            String userAlias,
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
                freshDataContext.userName,
                freshDataContext.userAlias,
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

