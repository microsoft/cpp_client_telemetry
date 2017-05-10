// Copyright (c) Microsoft. All rights reserved.

package com.microsoft.applications.telemetry;

/**
 * Semantic context interface.
 */
public interface ISemanticContext {

    /**
     * Set the application identifier context information of telemetry events.
     *
     * @param appId Id that uniquely identifies the user-facing App which
     *              the events originate from.
     */
    public void setAppId(String appId);
}
