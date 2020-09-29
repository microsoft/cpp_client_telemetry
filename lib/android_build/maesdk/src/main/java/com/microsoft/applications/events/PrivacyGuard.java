package com.microsoft.applications.events;

public class PrivacyGuard {

    //Initialize PG
    private static native void nativeInitializePrivacyGuard(
            long iLoggerNativePtr,
            String domainName,
            String machineName,
            String userName,
            String userAlias,
            Object[] ipAddresses,
            Object[] languageIdentifiers,
            Object[] machineIds,
            Object[] outOfScopeIdentifiers);

    private static native void nativeInitializePrivacyGuardWithoutCommonDataContext(long iLoggerNativePtr);
    /**
     * Initialize Privacy Guard from Logger
     * @param loggerNativePtr Native Ptr to ILogger, only accessible in Logger.
     * @param dataContext Common Data Context to initialize Privacy Guard with.
     */
    /*package-private*/ static void initializePrivacyGuardFromLogger(long loggerNativePtr, CommonDataContext dataContext)
    {
        if(dataContext != null)
        {
            nativeInitializePrivacyGuard(loggerNativePtr,
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
            nativeInitializePrivacyGuardWithoutCommonDataContext(loggerNativePtr);
        }
    }

    /**
     * Set the Enabled state for Privacy Guard
     * @param isEnabled New Enabled value
     */
    public static native void setEnabled(boolean isEnabled);

    /**
     * Get the Enabled state for Privacy Guard
     * @return `true` is Privacy Guard is initialized and enabled, `false` otherwise.
     */
    public static native boolean isEnabled();

    private static native void nativeAppendCommonDataContext(
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
     */
    public static void appendCommonDataContext(CommonDataContext freshDataContext)
    {
        if(freshDataContext == null)
        {
            return;
        }

        nativeAppendCommonDataContext(
                freshDataContext.domainName,
                freshDataContext.machineName,
                freshDataContext.userName,
                freshDataContext.userAlias,
                freshDataContext.ipAddresses.toArray(),
                freshDataContext.languageIdentifiers.toArray(),
                freshDataContext.machineIds.toArray(),
                freshDataContext.outOfScopeIdentifiers.toArray());
    }

    private static native void nativeAddIgnoredConcern(String eventName, String fieldName, int dataConcern);

    /**
     * Add a known concern to be ignored.
     * @param eventName EventName that might contain a concern. <b>Note:</b> If the ignored concern applies to Semantic Context field, set the Event name to 'SemanticContext'.
     * @param fieldName FieldName that might contain a concern.
     * @param dataConcern Specific DataConcernType you wish to ignore for the given event and field combination.
     */
    public static void addIgnoredConcern(String eventName, String fieldName, DataConcernType dataConcern)
    {
        nativeAddIgnoredConcern(eventName, fieldName, dataConcern.getValue());
    }
}
