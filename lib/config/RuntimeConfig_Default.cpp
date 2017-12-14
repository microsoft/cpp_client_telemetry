// Copyright (c) Microsoft. All rights reserved.

#include "RuntimeConfig_Default.hpp"

namespace ARIASDK_NS_BEGIN {


RuntimeConfig_Default::RuntimeConfig_Default()
{
}

RuntimeConfig_Default::~RuntimeConfig_Default()
{
}

void RuntimeConfig_Default::initialize(LogConfiguration& configuration)
{
    EVTStatus error;
    std::string url = configuration.GetProperty(CFG_STR_COLLECTOR_URL, error);
    if (!url.empty()) {
        m_collectorUrl = url;
    }
    int cacheFileSizeLimitInBytes = configuration.GetIntProperty(CFG_INT_CACHE_FILE_SIZE, error);
    if (cacheFileSizeLimitInBytes != 0 && error  == false) {
        m_offlineStorageMaximumSize = cacheFileSizeLimitInBytes;
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

EventLatency RuntimeConfig_Default::GetEventLatency(std::string const& tenantId, std::string const& eventName) const
{
	UNREFERENCED_PARAMETER(tenantId);
	UNREFERENCED_PARAMETER(eventName);
    return EventLatency_Unspecified;
}

std::string RuntimeConfig_Default::GetMetaStatsTenantToken() const
{
	std::size_t found = m_collectorUrl.find(".int.");
	if (found != std::string::npos)
	{
		return m_metaStatsTenantToken_INT;
	}

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
    return false;// m_isHttpRequestCompressionEnabled;
}

unsigned RuntimeConfig_Default::GetMinimumUploadBandwidthBps() const
{
    return m_minimumUploadBandwidthBps;
}

unsigned RuntimeConfig_Default::GetMaximumUploadSizeBytes() const
{
    return m_maximumUploadSizeBytes;
}

void RuntimeConfig_Default::SetEventLatency(std::string const& tenantId, std::string const& eventName, EventLatency latency)
{
	UNREFERENCED_PARAMETER(tenantId);
	UNREFERENCED_PARAMETER(eventName);
	UNREFERENCED_PARAMETER(latency);
}

bool RuntimeConfig_Default::IsClockSkewEnabled() const
{
	return true;
}

} ARIASDK_NS_END
