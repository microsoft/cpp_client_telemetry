// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

import android.util.Log;

import java.util.HashMap;


// Maybe this file could be (eventually) split into some smaller, logical chunks.
// With the current amount of methods it is probably better to keep it like this.


/**
 * Proxy to the native Aria library.
 * This is the only class that calls native methods and is "internal" (i.e.
 * not part of the API and available only to classes in this package).
 */
class AriaProxy {

    private static final String LOG_TAG = AriaProxy.class.getSimpleName();

    private String tenantToken = null;

    /**
     * Return if the native LogManager has been initialized.
     */
    public static boolean isInitialized() {
        return isInitializedNative();
    }

    /**
     * Set global context fields.
     * All (used) context fields are set at once to save multiple calls across JNI.
     */
    public static void setContextFields(String appId, String appLanguage, String osBuild, String timeZone) {
        Log.i(LOG_TAG, String.format("setContextFields(appId=%s, appLanguage=%s, osBuild=%s, timeZone=%s)", appId, appLanguage, osBuild, timeZone));
        setContextFieldsNative(appId, appLanguage, osBuild, timeZone);
    }

    /**
     * Construct AriaProxy.
     * The tenant token is stored inside so that a native Logger can be created
     * when needed.
     *
     * @param tenantToken Aria tenant token.
     */
    public AriaProxy(String tenantToken) {
        if (tenantToken == null) {
            throw new NullPointerException("tenantToken can't be null");
        }
        this.tenantToken = tenantToken;
    }

    /**
     * Set AppId on Logger's SemanticContext.
     *
     * @param appID See documentation of ISemanticContext.
     */
    public void setAppId(String appId) {
        if (tenantToken == null) {
            Log.e(LOG_TAG, "setAppId can be only called with tenantToken");
            return;
        }

        if (!setAppIdForLoggerNative(tenantToken, appId)) {
            Log.e(LOG_TAG, "Failed to set AppId for Logger");
        }
    }

    /**
     * Log an event.
     * Sends the contents of EventProperties to native C++ methods.
     */
    public void logEvent(EventProperties properties) {
        long nativeProperties = createEventPropertiesNative(properties.name, properties.priority.getValue());
        if (nativeProperties == 0) {
            Log.e(LOG_TAG, String.format("Failed to create native event: %s", properties.name));
            return;
        }

        for (HashMap.Entry<String, HashMap.SimpleEntry<String, PiiKind>> entry : properties.properties.entrySet()) {
            HashMap.Entry<String, PiiKind> pair = entry.getValue();
            setPropertyNative(nativeProperties, entry.getKey(), pair.getKey(), pair.getValue().getValue());
        }

        // Working with tenant tokens instead of native Logger pointers to save a call
        // across the JNI boundary for first retrieving the Logger, then logging the event.
        if (!logEventAndDeleteEventPropertiesNative(this.tenantToken, nativeProperties)) {
            deleteEventPropertiesNative(nativeProperties);
            Log.e(LOG_TAG, String.format("Failed to log event: %s", properties.name));
        }
    }


    // Methods for SemanticContext
    public static native boolean setAppIdForLoggerNative(String tenantToken, String appId);

    // Methods for Logger/logEvent()
    // Deviating from the outward API to save one call accross the JNI boundary (setPriority).
    private static native long createEventPropertiesNative(String eventName, int priority);
    private static native boolean logEventAndDeleteEventPropertiesNative(String tenantToken, long eventProperties);
    private static native void setPropertyNative(long eventPropertiesInstance, String name, String value, int piiKind);
    private static native void deleteEventPropertiesNative(long eventProperties);

    // Methods for LogManager (global)
    private static native boolean isInitializedNative();
    public static native void setContextFieldsNative(String appId, String appLanguage, String osBuild, String timeZone);
}
