// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/IRuntimeConfig.hpp>
#include <aria/LogConfiguration.hpp>

namespace ARIASDK_NS_BEGIN {


class RuntimeConfig_Default : public IRuntimeConfig {
  public:
    RuntimeConfig_Default();
    RuntimeConfig_Default(RuntimeConfig_Default const&) = delete;
    RuntimeConfig_Default& operator=(RuntimeConfig_Default const&) = delete;
    virtual ~RuntimeConfig_Default();

    void initialize(LogConfiguration const& configuration);

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
	virtual bool IsClockSkewEnabled() const override;

  protected:
    std::string m_collectorUrl                    = "https://mobile.pipe.aria.microsoft.com/Collector/3.0/";
	std::string m_metaStatsTenantToken            = "ead4d35d9f17486581d6c09afbe41263-01b1a12d-a157-460b-8efd-af9a10b09682-7259";
    std::string m_uploadRetryBackoffConfig        = "E,3000,300000,2,1";
    unsigned    m_metaStatsSendIntervalSec        = 300;
    unsigned    m_offlineStorageMaximumSize       = 3 * 1024 * 1024;
    unsigned    m_offlineStorageResizeThreshold   = 20;
    unsigned    m_maxRetryCount                   = 5;
    unsigned    m_minimumUploadBandwidthBps       = 512;
    unsigned    m_maximumUploadSizeBytes          = 1 * 1024 * 1024;
    bool        m_isHttpRequestCompressionEnabled = true;
};


} ARIASDK_NS_END
