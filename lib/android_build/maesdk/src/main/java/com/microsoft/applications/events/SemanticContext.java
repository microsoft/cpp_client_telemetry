//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

class SemanticContext implements ISemanticContext {
    private final long m_nativeISemanticContextPtr;

    /**
     * @return native pointer of the ISemanticContext object
     */
    long getNativeSemanticContextPtr() {
        return m_nativeISemanticContextPtr;
    }

    /**
     * Construct java SemanticContext based on the native SemanticContext pointer to help the jni method calls.
     * No consumer of 1DS sdk should have a need to call this constructor.
     *
     * @param nativeISemanticContextPtr
     */
    SemanticContext(final long nativeISemanticContextPtr) {
        m_nativeISemanticContextPtr = nativeISemanticContextPtr;
    }

    /**
     * Set the application environment context information of telemetry event.
     *
     * @param appEnv Environment from which this event originated
     */
    @Override
    public void setAppEnv(final String appEnv) {
        setCommonField(Constants.COMMONFIELDS_APP_ENV, appEnv);
    }

    /**
     * Set the application identifier context information of telemetry event.
     *
     * @param appId Id that uniquely identifies the user-facing application from which this event originated
     */
    @Override
    public void setAppId(final String appId) {
        setCommonField(Constants.COMMONFIELDS_APP_ID, appId);
    }

    /**
     * Set the application name context information of telemetry event.
     *
     * @param appName Application Name
     */
    @Override
    public void setAppName(final String appName) {
        setCommonField(Constants.COMMONFIELDS_APP_NAME, appName);
    }

    /**
     * Set the application version context information of telemetry event.
     *
     * @param appVersion Version of the application, retrieved programmatically where possible and is app/platform specific
     */
    @Override
    public void setAppVersion(final String appVersion) {
        setCommonField(Constants.COMMONFIELDS_APP_VERSION, appVersion);
    }

    /**
     * Set the application language context information of telemetry event.
     *
     * @param appLanguage Application Language
     */
    @Override
    public void setAppLanguage(final String appLanguage) {
        setCommonField(Constants.COMMONFIELDS_APP_LANGUAGE, appLanguage);
    }

    /**
     * Set the application's experiment IDs information of telemetry event.
     * The experiment IDs information will be applied to all events unless it is overwritten by that set via SetEventExperimentIds
     *
     * @param appExperimentIds list of IDs of experiments into which the application is enlisted
     */
    @Override
    public void setAppExperimentIds(final String appExperimentIds) {
        setCommonField(Constants.COMMONFIELDS_APP_EXPERIMENTIDS, appExperimentIds);
    }

    private native void nativeSetAppExperimentETag(long nativeISemanticContextPtr, String appExperimentETag);

    /**
     * Set the application version context information of telemetry event.
     * Removes previously stored experiment ids set by SetAppExperimentIds.
     *
     * @param appExperimentETag
     */
    @Override
    public void setAppExperimentETag(final String appExperimentETag) {
        if (appExperimentETag == null || appExperimentETag.trim().isEmpty())
            throw new IllegalArgumentException("appExperimentETag is null or empty");

        nativeSetAppExperimentETag(m_nativeISemanticContextPtr, appExperimentETag);
    }

    /**
     * Set the application experimentation impression id information of telemetry event.
     *
     * @param appExperimentImpressionId List of experiment IDs which are app/platform specific
     */
    @Override
    public void setAppExperimentImpressionId(final String appExperimentImpressionId) {
        setCommonField(Constants.SESSION_IMPRESSION_ID, appExperimentImpressionId);
    }

    private native void nativeSetEventExperimentIds(long nativeISemanticContextPtr, String eventName, String experimentIds);

    /**
     * Set the experiment IDs information of the specified telemetry event.
     *
     * @param eventName
     * @param experimentIds
     */
    @Override
    public void setEventExperimentIds(final String eventName, final String experimentIds) {
        if (eventName == null || eventName.trim().isEmpty())
            throw new IllegalArgumentException("eventName is null or empty");
        if (experimentIds == null || experimentIds.trim().isEmpty())
            throw new IllegalArgumentException("experimentIds is null or empty");

        nativeSetEventExperimentIds(m_nativeISemanticContextPtr, eventName, experimentIds);
    }

    private native void nativeClearExperimentIds(long nativeISemanticContextPtr);

    /**
     * Clear the experiment IDs information.
     */
    @Override
    public void clearExperimentIds() {
        nativeClearExperimentIds(m_nativeISemanticContextPtr);
    }

    /**
     * Set the device identifier context information of telemetry event.
     *
     * @param deviceId A unique device identifier, retrieved programmatically where possible and is app/platform specific
     */
    @Override
    public void setDeviceId(final String deviceId) {
        setCommonField(Constants.COMMONFIELDS_DEVICE_ID, deviceId);
    }

    /**
     * Set the device identifier context information of telemetry event.
     *
     * @param deviceMake The manufacturer of the device, retrieved programmatically where possible and is app/platform specific
     */
    @Override
    public void setDeviceMake(final String deviceMake) {
        setCommonField(Constants.COMMONFIELDS_DEVICE_MAKE, deviceMake);
    }

    /**
     * Set the device model context information of telemetry event.
     *
     * @param deviceModel The model of the device, retrieved programmatically where possible and is app/platform specific
     */
    @Override
    public void setDeviceModel(final String deviceModel) {
        setCommonField(Constants.COMMONFIELDS_DEVICE_MODEL, deviceModel);
    }

    /**
     * Set the device class context information of telemetry event.
     *
     * @param deviceClass Device class.
     */
    @Override
    public void setDeviceClass(final String deviceClass) {
        setCommonField(Constants.COMMONFIELDS_DEVICE_CLASS, deviceClass);
    }

    private native void nativeSetNetworkCost(long nativeISemanticContextPtr, int networkCost);

    /**
     * Set the network cost context information of telemetry event.
     * @param networkCost The cost of using data traffic on the current network
     */
    @Override
    public void setNetworkCost(final NetworkCost networkCost) {
        if (networkCost == null)
            throw new IllegalArgumentException("networkCost is null");

        nativeSetNetworkCost(m_nativeISemanticContextPtr, networkCost.getValue());
    }

    /**
     * Set the network provider context information of telemetry event.
     *
     * @param networkProvider The provider used to connect to the current network, retrieved programmatically where possible and is app/platform specific
     */
    @Override
    public void setNetworkProvider(final String networkProvider) {
        setCommonField(Constants.COMMONFIELDS_NETWORK_PROVIDER, networkProvider);
    }

    private native void nativeSetNetworkType(long nativeISemanticContextPtr, int networkType);

    /**
     * Set the network type context information of telemetry event.
     * @param networkType The type of the current network
     */
    @Override
    public void SetNetworkType(final NetworkType networkType) {
        if (networkType == null)
            throw new IllegalArgumentException("networkType is null");

        nativeSetNetworkType(m_nativeISemanticContextPtr, networkType.getValue());
    }

    /**
     * Set the system name context information of telemetry event.
     *
     * @param osName The system name, retrieved programmatically where possible and is app/platform specific
     */
    @Override
    public void setOsName(final String osName) {
        setCommonField(Constants.COMMONFIELDS_OS_NAME, osName);
    }

    /**
     * Set the system version context information of telemetry event.
     *
     * @param osVersion The system version, retrieved programmatically where possible and is app/platform specific
     */
    @Override
    public void setOsVersion(final String osVersion) {
        setCommonField(Constants.COMMONFIELDS_OS_VERSION, osVersion);
    }

    /**
     * Set the system build number context information of telemetry event.
     *
     * @param osBuild The system build, retrieved programmatically where possible and is app/platform specific
     */
    @Override
    public void setOsBuild(final String osBuild) {
        setCommonField(Constants.COMMONFIELDS_OS_BUILD, osBuild);
    }

    /**
     * Set the userId context information of telemetry event and default to PiiKind_Identity.
     *
     * @param userId Identifier that uniquely identifies a user in the application-specific user namespace
     */
    @Override
    public void setUserId(final String userId) {
        setUserId(userId, PiiKind.Identity);
    }

    private native void nativeSetUserId(long nativeISemanticContextPtr, String userId, int PiiKind);

    /**
     * Set the userId context information of telemetry event.
     *
     * @param userId Identifier that uniquely identifies a user in the application-specific user namespace
     * @param piiKind PIIKind of the userId. Default to PiiKind_Identity, set it to PiiKind_None to denote it as non-PII.
     */
    @Override
    public void setUserId(final String userId, final PiiKind piiKind) {
        if (userId == null || userId.trim().isEmpty())
            throw new IllegalArgumentException("userId is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind_Identity is null");

        nativeSetUserId(m_nativeISemanticContextPtr, userId, piiKind.getValue());
    }

    /**
     * Set the user MsaId context information of telemetry event.
     *
     * @param userMsaId Msa id that identifies a user in the application-specific user namespace
     */
    @Override
    public void setUserMsaId(final String userMsaId) {
        setCommonField(Constants.COMMONFIELDS_USER_MSAID, userMsaId);
    }

    /**
     * Set the user ANID context information of telemetry event.
     *
     * @param userANID ANID that identifies a user in in the application-specific user namespace
     */
    @Override
    public void setUserANID(final String userANID) {
        setCommonField(Constants.COMMONFIELDS_USER_ANID, userANID);
    }

    /**
     * Set the advertising Id context information of telemetry event.
     *
     * @param userAdvertisingId Advertising Id of a user to use in an application-specific user namespace
     */
    @Override
    public void setUserAdvertisingId(final String userAdvertisingId) {
        setCommonField(Constants.COMMONFIELDS_USER_ADVERTISINGID, userAdvertisingId);
    }

    /**
     * Set the user language context information of telemetry event.
     *
     * @param userLanguage user's language in IETF language tag format, as described in RFC 4646.
     */
    @Override
    public void setUserLanguage(final String userLanguage) {
        setCommonField(Constants.COMMONFIELDS_USER_LANGUAGE, userLanguage);
    }

    /**
     * Set the user time zone context information of telemetry event.
     *
     * @param userTimeZone user's time zone relative to UTC, in ISO 8601 time zone format
     */
    @Override
    public void setUserTimeZone(final String userTimeZone) {
        setCommonField(Constants.COMMONFIELDS_USER_TIMEZONE, userTimeZone);
    }

    /**
     * Set the Commercial Id context information of telemetry event.
     *
     * @param commercialId CommercialId of a machine
     */
    @Override
    public void setCommercialId(final String commercialId) {
        setCommonField(Constants.COMMONFIELDS_COMMERCIAL_ID, commercialId);
    }

    private native void nativeSetCommonFieldString(long nativeISemanticContextPtr, String name, String value);

    /**
     * Sets the common Part A/B field.
     *
     * @param name Field name
     * @param value Field value in string
     */
    @Override
    public void setCommonField(final String name, final String value) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");

        nativeSetCommonFieldString(m_nativeISemanticContextPtr, name, value);
    }

    private native void nativeSetCommonField(long nativeISemanticContextPtr, String name, EventProperty value);

    /**
     * Sets the common Part A/B field.
     *
     * @param name Field name
     * @param value Field value
     */
    @Override
    public void setCommonField(final String name, final EventProperty value) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");

        nativeSetCommonField(m_nativeISemanticContextPtr, name, value);
    }

    private native void nativeSetCustomFieldString(long nativeISemanticContextPtr, String name, String value);

    /**
     * Sets the custom Part C field.
     *
     * @param name Field name
     * @param value Field value in string
     */
    @Override
    public void setCustomField(final String name, final String value) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");

        nativeSetCustomFieldString(m_nativeISemanticContextPtr, name, value);
    }

    private native void nativeSetCustomField(long nativeISemanticContextPtr, String name, EventProperty value);

    /**
     * Sets the custom Part C field.
     *
     * @param name Field name
     * @param value Field value
     */
    @Override
    public void setCustomField(final String name, final EventProperty value) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");

        nativeSetCustomField(m_nativeISemanticContextPtr, name, value);
    }

    private native void nativeSetTicket(long nativeISemanticContextPtr, int ticketType, String ticketValue);

    /**
     * Sets the ticket (device ticket, user id ticket, etc.) for secure token validation.
     * @param ticketType Ticket type
     * @param ticketValue Ticket value
     */
    @Override
    public void setTicket(final TicketType ticketType, final String ticketValue) {
        if (ticketValue == null || ticketValue.trim().isEmpty())
            throw new IllegalArgumentException("ticketValue is null or empty");
        if (ticketType == null)
            throw new IllegalArgumentException("ticketType is null");

        nativeSetTicket(m_nativeISemanticContextPtr, ticketType.getValue(), ticketValue);
    }
}

