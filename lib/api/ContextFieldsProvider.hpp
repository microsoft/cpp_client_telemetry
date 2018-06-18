// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <ILogger.hpp>
#include <mutex>
#include "bond/generated/AriaProtocol_types.hpp"

namespace ARIASDK_NS_BEGIN {


    class ContextFieldsProvider : public ISemanticContext
    {
    protected:
        typedef std::map<std::string, std::string>         KeyValueMap;
        typedef std::map<std::string, ::AriaProtocol::PII> PiiKeyValueMap;

    public:
        ContextFieldsProvider();
        ContextFieldsProvider(ContextFieldsProvider* parent);
        ContextFieldsProvider(ContextFieldsProvider const& copy);
        ContextFieldsProvider& operator=(ContextFieldsProvider const& copy);
        virtual ~ContextFieldsProvider();

        void setCustomField(std::string const& name, EventProperty value);
        void writeToRecord(::AriaProtocol::Record& record);
        void setCommonField(std::string const& name, EventProperty value);

    public:
        // ISemanticContext implementation
        virtual void SetAppId(std::string const& appId) override;
        virtual void SetAppExperimentIds(std::string const& appExperimentIds) override;
        virtual void SetAppExperimentETag(std::string const& appExperimentETag)override;
        virtual void SetAppExperimentImpressionId(std::string const& appExperimentImpressionId)override;
        virtual void SetEventExperimentIds(std::string const& eventName, std::string const& experimentIds)override;
        virtual void SetAppLanguage(std::string const& appLanguage) override;
        virtual void SetAppVersion(std::string const& appVersion) override;

        virtual void SetDeviceId(std::string const& deviceId) override;
        virtual void SetDeviceMake(std::string const& deviceMake) override;
        virtual void SetDeviceModel(std::string const& deviceModel) override;
        virtual void SetDeviceClass(std::string const& deviceCLass) override;

        virtual void SetNetworkCost(NetworkCost networkCost) override;
        virtual void SetNetworkProvider(std::string const& networkProvider) override;
        virtual void SetNetworkType(NetworkType networkType) override;

        virtual void SetOsBuild(std::string const& osBuild) override;
        virtual void SetOsName(std::string const& osName) override;
        virtual void SetOsVersion(std::string const& osVersion) override;

        virtual void SetUserId(std::string const& userId, PiiKind piiKind = PiiKind_Identity) override;
        virtual void SetUserMsaId(std::string const& userMsaId) override;
        virtual void SetUserANID(std::string const& userANID) override;
        virtual void SetUserAdvertisingId(std::string const& userAdvertingId) override;
        virtual void SetUserLanguage(std::string const& language) override;
        virtual void SetUserTimeZone(std::string const& timeZone) override;
        virtual void SetTicket(TicketType type, std::string const& ticketValue) override;

    protected:
        void _ClearExperimentIds();

        std::mutex              m_lock;
        ContextFieldsProvider*  m_parent;

        std::map<std::string, EventProperty>*  m_commonContextFieldsP;      // common context fields
        std::map<std::string, EventProperty>*  m_customContextFieldsP;      // custom context fields

        // mapping from an event name to a list of CSV'ed ECS configIds
        std::map<std::string, std::string>* m_commonContextEventToConfigIdsP;

        std::string* m_CommonFieldsAppExperimentIdsP;
        std::map<TicketType, std::string>* m_ticketsMapP;
    };


} ARIASDK_NS_END
