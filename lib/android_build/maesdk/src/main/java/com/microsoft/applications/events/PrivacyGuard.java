package com.microsoft.applications.events;

import com.microsoft.applications.events.CommonDataContext;
import com.microsoft.applications.events.DataConcernType;
import com.microsoft.applications.events.Logger;

import java.util.Vector;

public class PrivacyGuard {

    private static String[] vectorToStringArray(Vector<String> vectorToConvert)
    {
        String[] strings = new String[vectorToConvert.size()];
        int index = 0;
        for(String str : vectorToConvert)
        {
            strings[index++] = str;
        }

        return strings;
    }

    //Initialize PG
    private static native void nativeInitializePrivacyGuard(
            long iLoggerNativePtr,
            String domainName,
            String machineName,
            String userName,
            String userAlias,
            String[] ipAddresses,
            String[] languageIdentifiers,
            String[] machineIds,
            String[] outOfScopeIdentifiers);

    public static void InitializePrivacyGuard(long loggerNativePtr, CommonDataContext dataContext)
    {
        nativeInitializePrivacyGuard(loggerNativePtr,
                dataContext.DomainName,
                dataContext.MachineName,
                dataContext.UserName,
                dataContext.UserAlias,
                vectorToStringArray(dataContext.IpAddresses),
                vectorToStringArray(dataContext.LanguageIdentifiers),
                vectorToStringArray(dataContext.MachineIds),
                vectorToStringArray(dataContext.OutOfScopeIdentifiers));

    }

    //SetEnabled
    private static native void nativeSetEnabled(boolean isEnabled);
    public static void SetEnabled(boolean isEnabled)
    {
        nativeSetEnabled(isEnabled);
    }

    //IsEnabled
    private static native boolean nativeIsEnabled();
    public static boolean IsEnabled()
    {
        return nativeIsEnabled();
    }
    //AppendCommonDataContext
    private static native void nativeAppendCommonDataContext(
            String domainName,
            String machineName,
            String userName,
            String userAlias,
            String[] ipAddresses,
            String[] languageIdentifiers,
            String[] machineIds,
            String[] outOfScopeIdentifiers);

    public static void AppendCommonDataContext(CommonDataContext dataContext)
    {
        nativeAppendCommonDataContext(
                dataContext.DomainName,
                dataContext.MachineName,
                dataContext.UserName,
                dataContext.UserAlias,
                vectorToStringArray(dataContext.IpAddresses),
                vectorToStringArray(dataContext.LanguageIdentifiers),
                vectorToStringArray(dataContext.MachineIds),
                vectorToStringArray(dataContext.OutOfScopeIdentifiers));
    }

    //AddIgnoredConcern
    private static native void nativeAddIgnoredConcern(String eventName, String fieldName, int dataConcern);

    public static void AddIgnoredConcern(String eventName, String fieldName, DataConcernType dataConcern)
    {
        nativeAddIgnoredConcern(eventName, fieldName, dataConcern.getValue());
    }
}
