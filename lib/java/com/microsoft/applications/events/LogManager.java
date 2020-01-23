package com.microsoft.applications.events;

public class LogManager {
    private LogManager() {};

    private static native long initalizeWithoutTenantToken();

    public static long initialize(){
        return initalizeWithoutTenantToken();
    }

    private static native long initializeWithTenantToken(String tenantToken);

    public static long initialize(String tenantToken) {
        return initializeWithTenantToken(tenantToken);
    }
}
