// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

public class SemanticContext implements ISemanticContext {
    private AriaProxy proxy = null;

    public SemanticContext(AriaProxy proxy) {
        this.proxy = proxy;
    }

    /**
     * Set the application identifier context information of telemetry events.
     *
     * @param appId Id that uniquely identifies the user-facing App which
     *        the events originate from.
     */
    public void setAppId(String appId) {
        this.proxy.setAppId(appId);
    }
}
