//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public interface ISemanticContext {
    /**
     * Set the application environment context information of telemetry event.
     *
     * @param appEnv Environment from which this event originated
     */
    public void setAppEnv(final String appEnv);

    /**
     * Set the application identifier context information of telemetry event.
     *
     * @param appId Id that uniquely identifies the user-facing application from which this event originated
     */
    public void setAppId(final String appId);

    /**
     * Set the application name context information of telemetry event.
     *
     * @param appName Application Name
     */
    public void setAppName(final String appName);

    /**
     * Set the application version context information of telemetry event.
     *
     * @param appVersion Version of the application, retrieved programmatically where possible and is app/platform specific
     */
    public void setAppVersion(final String appVersion);

    /**
     * Set the application language context information of telemetry event.
     *
     * @param appLanguage Application Language
     */
    public void setAppLanguage(final String appLanguage);

    /**
     * Set the application's experiment IDs information of telemetry event.
     * The experiment IDs information will be applied to all events unless it is overwritten by that set via SetEventExperimentIds
     *
     * @param appExperimentIds list of IDs of experiments into which the application is enlisted
     */
    public void setAppExperimentIds(final String appExperimentIds);

    /**
     * Set the application version context information of telemetry event.
     * Removes previously stored experiment ids set by SetAppExperimentIds.
     *
     * @param appExperimentETag
     */
    public void setAppExperimentETag(final String appExperimentETag);

    /**
     * Set the application experimentation impression id information of telemetry event.
     *
     * @param appExperimentImpressionId List of experiment IDs which are app/platform specific
     */
    public void setAppExperimentImpressionId(final String appExperimentImpressionId);

    /**
     * Set the experiment IDs information of the specified telemetry event.
     *
     * @param eventName
     * @param experimentIds
     */
    public void setEventExperimentIds(final String eventName, final String experimentIds);

    /**
     * Clear the experiment IDs information.
     */
    public void clearExperimentIds();

    /**
     * Set the device identifier context information of telemetry event.
     *
     * @param deviceId A unique device identifier, retrieved programmatically where possible and is app/platform specific
     */
    public void setDeviceId(final String deviceId);

    /**
     * Set the device identifier context information of telemetry event.
     *
     * @param deviceMake The manufacturer of the device, retrieved programmatically where possible and is app/platform specific
     */
    public void setDeviceMake(final String deviceMake);

    /**
     * Set the device model context information of telemetry event.
     *
     * @param deviceModel The model of the device, retrieved programmatically where possible and is app/platform specific
     */
    public void setDeviceModel(final String deviceModel);

    /**
     * Set the device class context information of telemetry event.
     *
     * @param deviceClass Device class.
     */
    public void setDeviceClass(final String deviceClass);

    /**
     * Set the network cost context information of telemetry event.
     * @param networkCost The cost of using data traffic on the current network
     */
    public void setNetworkCost(final NetworkCost networkCost);

    /**
     * Set the network provider context information of telemetry event.
     *
     * @param networkProvider The provider used to connect to the current network, retrieved programmatically where possible and is app/platform specific
     */
    public void setNetworkProvider(final String networkProvider);

    /**
     * Set the network type context information of telemetry event.
     * @param networkType The type of the current network
     */
    public void SetNetworkType(final NetworkType networkType);

    /**
     * Set the system name context information of telemetry event.
     *
     * @param osName The system name, retrieved programmatically where possible and is app/platform specific
     */
    public void setOsName(final String osName);

    /**
     * Set the system version context information of telemetry event.
     *
     * @param osVersion The system version, retrieved programmatically where possible and is app/platform specific
     */
    public void setOsVersion(final String osVersion);

    /**
     * Set the system build number context information of telemetry event.
     *
     * @param osBuild The system build, retrieved programmatically where possible and is app/platform specific
     */
    public void setOsBuild(final String osBuild);

    /**
     * Set the userId context information of telemetry event and default to PiiKind_Identity.
     *
     * @param userId Identifier that uniquely identifies a user in the application-specific user namespace
     */
    public void setUserId(final String userId);

    /**
     * Set the userId context information of telemetry event.
     * @param userId Identifier that uniquely identifies a user in the application-specific user namespace
     * @param piiKind_Identity PIIKind of the userId. Default to PiiKind_Identity, set it to PiiKind_None to denote it as non-PII.
     */
    public void setUserId(final String userId, final PiiKind piiKind_Identity);

    /**
     * Set the user MsaId context information of telemetry event.
     *
     * @param userMsaId Msa id that identifies a user in the application-specific user namespace
     */
    public void setUserMsaId(final String userMsaId);

    /**
     * Set the user ANID context information of telemetry event.
     *
     * @param userANID ANID that identifies a user in in the application-specific user namespace
     */
    public void setUserANID(final String userANID);

    /**
     * Set the advertising Id context information of telemetry event.
     *
     * @param userAdvertisingId Advertising Id of a user to use in an application-specific user namespace
     */
    public void setUserAdvertisingId(final String userAdvertisingId);

    /**
     * Set the user language context information of telemetry event.
     *
     * @param userLanguage user's language in IETF language tag format, as described in RFC 4646.
     */
    public void setUserLanguage(final String userLanguage);

    /**
     * Set the user time zone context information of telemetry event.
     *
     * @param userTimeZone user's time zone relative to UTC, in ISO 8601 time zone format
     */
    public void setUserTimeZone(final String userTimeZone);

    /**
     * Set the Commercial Id context information of telemetry event.
     *
     * @param commercialId CommercialId of a machine
     */
    public void setCommercialId(final String commercialId);

    /**
     * Sets the common Part A/B field.
     *
     * @param name Field name
     * @param value Field value in string
     */
    public void setCommonField(final String name, final String value);

    /**
     * Sets the common Part A/B field.
     * @param name Field name
     * @param value Field value
     */
    public void setCommonField(final String name, final EventProperty value);

    /**
     * Sets the custom Part C field.
     *
     * @param name Field name
     * @param value Field value
     */
    public void setCustomField(final String name, final String value);

    /**
     * Sets the custom Part C field.
     * @param name Field name
     * @param value Field value
     */
    public void setCustomField(final String name, final EventProperty value);

    /**
     * Sets the ticket (device ticket, user id ticket, etc.) for secure token validation.
     * @param ticketType Ticket type
     * @param ticketValue Ticket value
     */
    public void setTicket(final TicketType ticketType, final String ticketValue);
}

