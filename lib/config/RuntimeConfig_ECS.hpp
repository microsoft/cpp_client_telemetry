// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/IRuntimeConfig.hpp>
#include "pal/PAL.hpp"
#include <ecsClientInterface.hpp>
#include <map>

namespace ARIASDK_NS_BEGIN {


class RuntimeConfig_ECS : public IRuntimeConfig,
                          public ecsclient::IEcsCallback
{
  public:
    struct EventKey
    {
        std::string tenantId;
        std::string eventName;

        bool operator<(EventKey const& rhs) const
        {
            int res = tenantId.compare(rhs.tenantId);
            if (res == 0) {
                res = eventName.compare(rhs.eventName);
            }
            return (res < 0);
        }
    };

    typedef std::map<EventKey, EventPriority> EventPriorityMap;

  public:
    RuntimeConfig_ECS(ecsclient::IEcsClient& ecsClient, EventPriorityMap const& initialPriorityMap = EventPriorityMap());
    RuntimeConfig_ECS(RuntimeConfig_ECS const&) = delete;
    RuntimeConfig_ECS& operator=(RuntimeConfig_ECS const&) = delete;
    virtual ~RuntimeConfig_ECS();

  public:
    virtual void SetDefaultConfig(IRuntimeConfig& defaultConfig) override;
    virtual std::string GetCollectorUrl() const override;
    virtual void DecorateEvent(std::map<std::string, std::string>& extension, std::string const& experimentationProject, std::string const& eventName) const override;
    virtual EventPriority GetEventPriority(std::string const& tenantId = std::string(), std::string const& eventName = std::string()) const override;
    virtual std::string GetMetaStatsTenantToken() const override;
    virtual unsigned GetMetaStatsSendIntervalSec() const override;
    virtual unsigned GetOfflineStorageMaximumSizeBytes() const override;
    virtual unsigned GetOfflineStorageResizeThresholdPct() const override;
    virtual unsigned GetMaximumRetryCount() const override;
    virtual std::string GetUploadRetryBackoffConfig() const override;
    virtual bool IsHttpRequestCompressionEnabled() const override;
    virtual unsigned GetMinimumUploadBandwidthBps() const override;
    virtual unsigned GetMaximumUploadSizeBytes() const override;
    virtual void SetEventPriority(std::string const& tenantId, std::string const& eventName, EventPriority priority) override;

  public:
    virtual void OnEvent(CallbackEventType type, CallBackContext* context) override;

  protected:
    static EventPriority getPriorityFromMap(EventPriorityMap const& map, EventKey const& key);
    static EventPriority intToPriority(int value);
    void updateEcsConfiguration(ecsclient::IEcsConfigPtr const& config);

  protected:
    PAL::Mutex mutable                 m_lock;
    ecsclient::IEcsClient&             m_ecsClient;
    bool                               m_registered;
    IRuntimeConfig*                    m_defaultConfig;

    std::string                        m_etag;
    std::map<std::string, std::string> m_experimentIdsMap;

    EventPriorityMap                   m_apiPriorityMap;
    EventPriorityMap                   m_ecsPriorityMap;
    EventPriorityMap const             m_initialPriorityMap;

    std::string                        m_collectorUrl;
    std::string                        m_metaStatsTenantToken;
    std::string                        m_uploadRetryBackoffConfig;
    unsigned                           m_metaStatsSendIntervalSec;
    unsigned                           m_offlineStorageMaximumSize;
    unsigned                           m_offlineStorageResizeThreshold;
    unsigned                           m_maxRetryCount;
    unsigned                           m_minimumUploadBandwidthBps;
    unsigned                           m_maximumUploadSizeBytes;
    bool                               m_isHttpRequestCompressionEnabled;
};


} ARIASDK_NS_END
