// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

import android.util.Log;

/**
 * Implementation of ILogger for logging telemetry events.
 */
public class Logger implements ILogger {

    private static final String LOG_TAG = Logger.class.getSimpleName();

    private String tenantToken = null;
    private AriaProxy proxy = null;
    private SemanticContext context = null;

    /**
     * Constructor. An instance can be created only by LogManager.
     *
     * @param tenantToken Aria tenant token.
     * @see LogManager
     */
    protected Logger(String tenantToken) throws IllegalStateException {
        Log.v(LOG_TAG, String.format("Creating logger instance with tenantToken: %s", tenantToken));
        if (!AriaProxy.isInitialized()) {
            throw new IllegalStateException("Native Aria Client was not initialized. Unable to create Logger instance.");
        }
        this.tenantToken = tenantToken;
        this.proxy = new AriaProxy(tenantToken);
    }

    /**
     * Gets an ISemanticContext interface through which to specify semantic
     * context of this logger instance.
     *
     * @return An ISemanticContext instance..
     */
    public ISemanticContext getSemanticContext() {
        if (this.context == null) {
            this.context = new SemanticContext(this.proxy);
        }
        return this.context;
    }

    /**
     * Logs a custom event with specified name and fields to track information
     * such as how a particular feature is used.
     *
     * @param properties Prepared properties of the custom event.
     */
    public void logEvent(EventProperties properties) {
        Log.v(LOG_TAG, String.format("logEvent: event name: %s", properties.name));
            this.proxy.logEvent(properties);
    }
}
