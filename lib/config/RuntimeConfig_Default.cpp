// Copyright (c) Microsoft. All rights reserved.

#include "RuntimeConfig_Default.hpp"

namespace ARIASDK_NS_BEGIN {


RuntimeConfig_Default::RuntimeConfig_Default()
{
}

RuntimeConfig_Default::~RuntimeConfig_Default()
{
}

void RuntimeConfig_Default::initialize(LogConfiguration const& configuration)
{
    if (!configuration.eventCollectorUri.empty()) {
        m_collectorUrl = configuration.eventCollectorUri;
    }

    if (configuration.cacheFileSizeLimitInBytes != 0) {
        m_offlineStorageMaximumSize = configuration.cacheFileSizeLimitInBytes;
    }
}

void RuntimeConfig_Default::SetDefaultConfig(IRuntimeConfig& defaultConfig)
{
	UNREFERENCED_PARAMETER(defaultConfig);
}

std::string RuntimeConfig_Default::GetCollectorUrl() const
{
    return m_collectorUrl;
}

void RuntimeConfig_Default::DecorateEvent(std::map<std::string, std::string>& extension, std::string const& experimentationProject, std::string const& eventName) const
{
	UNREFERENCED_PARAMETER(extension);
	UNREFERENCED_PARAMETER(experimentationProject);
	UNREFERENCED_PARAMETER(eventName);
}

EventPriority RuntimeConfig_Default::GetEventPriority(std::string const& tenantId, std::string const& eventName) const
{
	UNREFERENCED_PARAMETER(tenantId);
	UNREFERENCED_PARAMETER(eventName);
    return EventPriority_Unspecified;
}

std::string RuntimeConfig_Default::GetMetaStatsTenantToken() const
{
    return m_metaStatsTenantToken;
}

unsigned RuntimeConfig_Default::GetMetaStatsSendIntervalSec() const
{
    return m_metaStatsSendIntervalSec;
}

unsigned RuntimeConfig_Default::GetOfflineStorageMaximumSizeBytes() const
{
    return m_offlineStorageMaximumSize;
}

unsigned RuntimeConfig_Default::GetOfflineStorageResizeThresholdPct() const
{
    return m_offlineStorageResizeThreshold;
}

unsigned RuntimeConfig_Default::GetMaximumRetryCount() const
{
    return m_maxRetryCount;
}

std::string RuntimeConfig_Default::GetUploadRetryBackoffConfig() const
{
    return m_uploadRetryBackoffConfig;
}

bool RuntimeConfig_Default::IsHttpRequestCompressionEnabled() const
{
    return m_isHttpRequestCompressionEnabled;
}

unsigned RuntimeConfig_Default::GetMinimumUploadBandwidthBps() const
{
    return m_minimumUploadBandwidthBps;
}

unsigned RuntimeConfig_Default::GetMaximumUploadSizeBytes() const
{
    return m_maximumUploadSizeBytes;
}

void RuntimeConfig_Default::SetEventPriority(std::string const& tenantId, std::string const& eventName, EventPriority priority)
{
	UNREFERENCED_PARAMETER(tenantId);
	UNREFERENCED_PARAMETER(eventName);
	UNREFERENCED_PARAMETER(priority);
}

bool RuntimeConfig_Default::IsClockSkewEnabled() const
{
	return true;
}

} ARIASDK_NS_END
