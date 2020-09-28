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

    /**
     * Initialize Privacy Guard from Logger
     * @param loggerNativePtr Native Ptr to ILogger, only accessible in Logger.
     * @param dataContext Common Data Context to initialize Privacy Guard with.
     */
    public static void InitializePrivacyGuardFromLogger(long loggerNativePtr, CommonDataContext dataContext)
    {
        nativeInitializePrivacyGuard(loggerNativePtr,
                dataContext.DomainName,
                dataContext.MachineName,
                dataContext.UserName,
                dataContext.UserAlias,
                dataContext.IpAddresses.toArray(),
                dataContext.LanguageIdentifiers.toArray(),
                dataContext.MachineIds.toArray(),
                dataContext.OutOfScopeIdentifiers.toArray());

    }

    private static native void nativeSetEnabled(boolean isEnabled);

    /**
     * Set the Enabled state for Privacy Guard
     * @param isEnabled New Enabled value
     */
    public static void SetEnabled(boolean isEnabled)
    {
        nativeSetEnabled(isEnabled);
    }

    private static native boolean nativeIsEnabled();

    /**
     * Get the Enabled state for Privacy Guard
     * @return `true` is Privacy Guard is initialized and enabled, `false` otherwise.
     */
    public static boolean IsEnabled()
    {
        return nativeIsEnabled();
    }

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
    public static void AppendCommonDataContext(CommonDataContext freshDataContext)
    {
        nativeAppendCommonDataContext(
                freshDataContext.DomainName,
                freshDataContext.MachineName,
                freshDataContext.UserName,
                freshDataContext.UserAlias,
                freshDataContext.IpAddresses.toArray(),
                freshDataContext.LanguageIdentifiers.toArray(),
                freshDataContext.MachineIds.toArray(),
                freshDataContext.OutOfScopeIdentifiers.toArray());
    }

    private static native void nativeAddIgnoredConcern(String eventName, String fieldName, int dataConcern);

    /**
     * Add a known concern to be ignored.
     * @param eventName EventName that might contain a concern. <b>Note:</b> If the ignored concern applies to Semantic Context field, set the Event name to 'SemanticContext'.
     * @param fieldName FieldName that might contain a concern.
     * @param dataConcern Specific DataConcernType you wish to ignore for the given event and field combination.
     */
    public static void AddIgnoredConcern(String eventName, String fieldName, DataConcernType dataConcern)
    {
        nativeAddIgnoredConcern(eventName, fieldName, dataConcern.getValue());
    }
}
