//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef ISEMANTICCONTEXT_HPP
#define ISEMANTICCONTEXT_HPP

#include "EventProperty.hpp"
#include "CommonFields.h"
#include "ctmacros.hpp"
#include "Enums.hpp"

#include <string>
#include <cassert>

namespace MAT_NS_BEGIN
{
    /// <summary>
    class  MATSDK_LIBABI ISemanticContext
    {
    public:
        virtual  ~ISemanticContext() {};

        /// <summary>
        /// Set the application environment context information of telemetry event.
        /// </summary>
        /// <param name="appEnv">Environment from which this event originated</param>
        DECLARE_COMMONFIELD(AppEnv, COMMONFIELDS_APP_ENV);

        /// <summary>
        /// Set the application identifier context information of telemetry event.
        /// </summary>
        /// <param name="appId">Id that uniquely identifies the user-facing application from which this event originated</param>
        DECLARE_COMMONFIELD(AppId, COMMONFIELDS_APP_ID);

        /// <summary>
        /// Set the application name context information of telemetry event.
        /// </summary>
        /// <param name="appName">Application Name</param>
        DECLARE_COMMONFIELD(AppName, COMMONFIELDS_APP_NAME);

        /// <summary>
        /// Set the application version context information of telemetry event.
        /// </summary>
        /// <param name="appVersion">Version of the application, retrieved programmatically where possible and is app/platform specific</param>
        DECLARE_COMMONFIELD(AppVersion, COMMONFIELDS_APP_VERSION);

        /// <summary>
        /// Set the application language context information of telemetry event.
        /// </summary>
        DECLARE_COMMONFIELD(AppLanguage, COMMONFIELDS_APP_LANGUAGE);

        /// <summary>
        /// Set the application's experiment IDs information of telemetry event.
        /// The experiment IDs information will be applied to all events unless it is overwritten by that set via SetEventExperimentIds  
        /// </summary>
        /// <param name="appVersion">list of IDs of experimentations into which the application is enlisted</param>
        DECLARE_COMMONFIELD(AppExperimentIds, COMMONFIELDS_APP_EXPERIMENTIDS);

        /// <summary>
        /// Set the application version context information of telemetry event.
        /// Removes previously stored experiment ids set by SetAppExperimentIds.
        /// </summary>
        /// <param name="appVersion">ETAG which is a hash of the set of experimentations into which the application is enlisted</param>
        virtual void  SetAppExperimentETag(std::string const& appExperimentETag)
        {
            SetCommonField(COMMONFIELDS_APP_EXPERIMENTETAG, appExperimentETag);
            ClearExperimentIds();
        };

        /// <summary>
        /// Set the application experimentation impression id information of telemetry event.
        /// </summary>
        /// <param name="appExperimentIds">List of expementation IDs which are app/platform specific</param>
        DECLARE_COMMONFIELD(AppExperimentImpressionId, SESSION_IMPRESSION_ID);

        /// <summary>
        /// Set the experiment IDs information of the specified telemetry event.
        /// </summary>
        /// <param name="appVersion">list of IDs of experimentations into which the application is enlisted</param>
        virtual void  SetEventExperimentIds(std::string const& /*eventName*/, std::string const& /*experimentIds*/) {};

        /// <summary>
        /// Clear the experiment IDs information.
        /// </summary>
        virtual void ClearExperimentIds() {};

        /// <summary>
        /// Set the device identifier context information of telemetry event.
        /// </summary>
        /// <param name="deviceId">A unique device identifier, retrieved programmatically where possible and is app/platform specific</param>
        DECLARE_COMMONFIELD(DeviceId, COMMONFIELDS_DEVICE_ID);

        /// <summary>
        /// Set the device manufacturer context information of telemetry event.
        /// </summary>
        /// <param name="deviceMake">The manufacturer of the device, retrieved programmatically where possible and is app/platform specific</param>
        DECLARE_COMMONFIELD(DeviceMake, COMMONFIELDS_DEVICE_MAKE);

        /// <summary>
        /// Set the device model context information of telemetry event.
        /// </summary>
        /// <param name="deviceModel">The model of the device, retrieved programmatically where possible and is app/platform specific</param>
        DECLARE_COMMONFIELD(DeviceModel, COMMONFIELDS_DEVICE_MODEL);

        /// <summary>
        /// Set the device class context information of telemetry event.
        /// </summary>
        /// <param name="deviceClass">Device class.</param>
        DECLARE_COMMONFIELD(DeviceClass, COMMONFIELDS_DEVICE_CLASS);

          /// <summary>
        /// Set the device orgId context information of telemetry event.
        /// </summary>
        /// <param name="deviceClass">Device orgId</param>
        DECLARE_COMMONFIELD(DeviceOrgId, COMMONFIELDS_DEVICE_ORGID);

        /// <summary>
        /// Set the network cost context information of telemetry event.
        /// </summary>
        /// <param name="networkCost">The cost of using data traffic on the current network</param>
        virtual void SetNetworkCost(NetworkCost networkCost)
        {
            char const* value;

            switch (networkCost) {
            case NetworkCost_Unknown:
                value = "Unknown";
                break;

            case NetworkCost_Unmetered:
                value = "Unmetered";
                break;

            case NetworkCost_Metered:
                value = "Metered";
                break;

            case NetworkCost_OverDataLimit:
                value = "OverDataLimit";
                break;

            default:
                assert(!"Unknown NetworkCost enum value");
                value = "";
                break;
            }

            SetCommonField(COMMONFIELDS_NETWORK_COST, value);
        }

        /// <summary>
        /// Set the network provider context information of telemetry event.
        /// </summary>
        /// <param name="networkProvider">The provider used to connect to the current network, retrieved programmatically where possible and is app/platform specific</param>
        DECLARE_COMMONFIELD(NetworkProvider, COMMONFIELDS_NETWORK_PROVIDER);

        /// Set the network type context information of telemetry event.
        /// </summary>
        /// <param name="networkType">The type of the current network</param>
        virtual void SetNetworkType(NetworkType networkType)
        {
            char const* value;

            switch (networkType) {
            case NetworkType_Unknown:
                value = "Unknown";
                break;

            case NetworkType_Wired:
                value = "Wired";
                break;

            case NetworkType_Wifi:
                value = "Wifi";
                break;

            case NetworkType_WWAN:
                value = "WWAN";
                break;

            default:
                assert(!"Unknown NetworkType enum value");
                value = "";
                break;
            }

            SetCommonField(COMMONFIELDS_NETWORK_TYPE, value);
        }

        /// <summary>
        /// Set the system name context information of telemetry event.
        /// </summary>
        /// <param name="osName">The system anme, retrieved programmatically where possible and is app/platform specific</param>
        DECLARE_COMMONFIELD(OsName, COMMONFIELDS_OS_NAME);

        /// <summary>
        /// Set the system version context information of telemetry event.
        /// </summary>
        /// <param name="osVersion">The system version, retrieved programmatically where possible and is app/platform specific</param>
        DECLARE_COMMONFIELD(OsVersion, COMMONFIELDS_OS_VERSION);

        /// <summary>
        /// Set the system build number context information of telemetry event.
        /// </summary>
        /// <param name="osBuild">The system build, retrieved programmatically where possible and is app/platform specific</param>
        DECLARE_COMMONFIELD(OsBuild, COMMONFIELDS_OS_BUILD);

        /// <summary>
        /// Set the userId context information of telemetry event.
        /// </summary>
        /// <param name="userId">Identifier that uniquely identifies a user in the application-specific user namespace</param>
        /// <param name='piiKind'>PIIKind of the userId. Default to PiiKind_Identity, set it to PiiKind_None to denote it as non-PII.</param>
        virtual void  SetUserId(std::string const& userId, PiiKind piiKind = PiiKind_Identity)
        {
            EventProperty prop(userId, piiKind);
            SetCommonField(COMMONFIELDS_USER_ID, prop);
        }

        /// <summary>
        /// Set the user MsaId context information of telemetry event.
        /// </summary>
        /// <param name="userMsaId">Msa id that identifies a user in the application-specific user namespace</param>
        DECLARE_COMMONFIELD(UserMsaId, COMMONFIELDS_USER_MSAID);

        /// <summary>
        /// Set the user ANID context information of telemetry event.
        /// </summary>
        /// <param name="userANID">ANID that identifies a user in in the application-specific user namespace</param>
        DECLARE_COMMONFIELD(UserANID, COMMONFIELDS_USER_ANID);

        /// <summary>
        /// Set the advertising Id context information of telemetry event.
        /// </summary>
        /// <param name="userAdvertingId">Advertising Id of a user to use in an application-specific user namespace</param>
        DECLARE_COMMONFIELD(UserAdvertisingId, COMMONFIELDS_USER_ADVERTISINGID);

        /// <summary>
        /// Set the user language context information of telemetry event.
        /// </summary>
        /// <param name="locale">user's language in IETF language tag format, as described in RFC 4646.</param>
        DECLARE_COMMONFIELD(UserLanguage, COMMONFIELDS_USER_LANGUAGE);

        /// <summary>
        /// Set the user time zone context information of telemetry event.
        /// </summary>
        /// <param name="timeZone">user's time zone relative to UTC, in ISO 8601 time zone format</param>
        DECLARE_COMMONFIELD(UserTimeZone, COMMONFIELDS_USER_TIMEZONE);

        /// <summary>
        /// Set the Commercial Id context information of telemetry event.
        /// </summary>
        /// <param name="commercialId">CommercialId of a machine</param>
        DECLARE_COMMONFIELD(CommercialId, COMMONFIELDS_COMMERCIAL_ID);

        /// <summary>
        /// Sets the common Part A/B field.
        /// </summary>
        /// <param name="name">Field name</param>
        /// <param name="value">Field value.</param>
        virtual void SetCommonField(const std::string &, const EventProperty &) {};

        /// <summary>
        /// Sets the custom Part C field.
        /// </summary>
        /// <param name="name">Field name</param>
        /// <param name="value">Field value.</param>
        virtual void SetCustomField(const std::string &, const EventProperty &) {};

        /// <summary>
        /// Sets the ticket (device ticket, user id ticket, etc.) for secure token validation.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticket value.</param>
        virtual void SetTicket(TicketType /*type*/, std::string const& /*ticketValue*/) {};
    };

} MAT_NS_END

#endif //ISEMANTICCONTEXT_H

