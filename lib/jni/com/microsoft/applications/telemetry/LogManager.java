// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

import java.util.TimeZone;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Locale;

import android.content.Context;
import android.util.Log;
import android.os.Build;

public enum LogManager {

    INSTANCE;

    private static final String LOG_TAG = LogManager.class.getSimpleName();

    /**
     * Initializes the telemetry logging system with default configuration.
     *
     * @param context     Android application context.
     * @param AppId       AppId of the application.
     * @param appLanguage Language of the application
     * @return Logger     instance instantiated with the default configuration.
     */
    public static void initializeWithSkyLib(Context context, String appId, String appLanguage) {
        Log.v(LOG_TAG, String.format("initializeWithSkyLib(context=%s, appId=%s, appLanguage=%s)", context, appId, appLanguage));
        AriaProxy.setContextFields(appId, appLanguage, Build.VERSION.INCREMENTAL, getCurrentTimezoneOffset());
    }

    /**
     * Initializes the telemetry logging system with default configuration.
     * For backwards compatibility only
     *
     * @param context Android application context.
     * @param AppId   AppId of the application.
     * @return Logger instance instantiated with the default configuration.
     */
    public static void initializeWithSkyLib(Context context, String appId) {
        initializeWithSkyLib(context, appId, null);
    }

    private static String getCurrentTimezoneOffset() {
        TimeZone tz = TimeZone.getDefault();
        Calendar cal = GregorianCalendar.getInstance(tz);
        int offsetInMillis = tz.getOffset(cal.getTimeInMillis());

        String offset = String.format(Locale.US, "%02d:%02d",
                                      Math.abs( offsetInMillis / 3600000),
                                      Math.abs((offsetInMillis /   60000) % 60));
        offset = (offsetInMillis >= 0 ? "+" : "-") + offset;

        return offset;
    }

    /**
     * Retrieves a Logger instance through which to log telemetry events.
     *
     * @param tenantToken Requested Aria tenant token to use.
     * @return ILogger instance.
     */
    public static ILogger getLogger(String tenantToken) {
        Log.v(LOG_TAG, String.format("getLogger(tenantToken=%s)", tenantToken));
        return new Logger(tenantToken);
    }
}
