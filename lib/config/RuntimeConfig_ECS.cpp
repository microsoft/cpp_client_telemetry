// Copyright (c) Microsoft. All rights reserved.

#include "RuntimeConfig_ECS.hpp"
#include "decorators/IDecorator.hpp"

namespace ARIASDK_NS_BEGIN {


RuntimeConfig_ECS::RuntimeConfig_ECS(ecsclient::IEcsClient& ecsClient, EventPriorityMap const& initialPriorityMap)
  : m_lock("AriaSDK/RuntimeConfig_ECS"),
    m_ecsClient(ecsClient),
    m_registered(false),
    m_initialPriorityMap(initialPriorityMap)
{
}

RuntimeConfig_ECS::~RuntimeConfig_ECS()
{
    if (m_registered) {
        m_ecsClient.RemoveListener(this);
    }
}

void RuntimeConfig_ECS::SetDefaultConfig(IRuntimeConfig& defaultConfig)
{
    m_defaultConfig = &defaultConfig;
    if (m_ecsClient.AddListener(this) == ecsclient::ECS_ERROR_OK) {
        m_registered = true;
    }
    updateEcsConfiguration(m_ecsClient.GetCurrentConfig());
}

std::string RuntimeConfig_ECS::GetCollectorUrl() const
{
    PAL::ScopedMutexLock guard(m_lock);
    return m_collectorUrl;
}

void RuntimeConfig_ECS::DecorateEvent(std::map<std::string, std::string>& extension, std::string const& experimentationProject, std::string const& eventName) const
{
    PAL::ScopedMutexLock guard(m_lock);

    IDecorator::setIfNotEmpty(extension, "AppInfo.EcsEtag", m_etag);

    auto const it = m_experimentIdsMap.find(eventName);
    if (it != m_experimentIdsMap.end()) {
        IDecorator::setIfNotEmpty(extension, "AppInfo.ExperimentIds", it->second);
    }

    // Useless for now
    static_cast<void>(experimentationProject);
}

EventPriority RuntimeConfig_ECS::GetEventPriority(std::string const& tenantId, std::string const& eventName) const
{
    PAL::ScopedMutexLock guard(m_lock);

    EventKey key{tenantId, eventName};
    EventPriority priority = getPriorityFromMap(m_ecsPriorityMap, key);

    if (priority == EventPriority_Unspecified) {
        priority = getPriorityFromMap(m_initialPriorityMap, key);
    }

    if (priority == EventPriority_Unspecified) {
        priority = getPriorityFromMap(m_apiPriorityMap, key);
    }

    return priority;
}

std::string RuntimeConfig_ECS::GetMetaStatsTenantToken() const
{
    PAL::ScopedMutexLock guard(m_lock);
    return m_metaStatsTenantToken;
}

unsigned RuntimeConfig_ECS::GetMetaStatsSendIntervalSec() const
{
    return m_metaStatsSendIntervalSec;
}

unsigned RuntimeConfig_ECS::GetOfflineStorageMaximumSizeBytes() const
{
    return m_offlineStorageMaximumSize;
}

unsigned RuntimeConfig_ECS::GetOfflineStorageResizeThresholdPct() const
{
    return m_offlineStorageResizeThreshold;
}

unsigned RuntimeConfig_ECS::GetMaximumRetryCount() const
{
    return m_maxRetryCount;
}

std::string RuntimeConfig_ECS::GetUploadRetryBackoffConfig() const
{
    return m_uploadRetryBackoffConfig;
}

bool RuntimeConfig_ECS::IsHttpRequestCompressionEnabled() const
{
    return m_isHttpRequestCompressionEnabled;
}

unsigned RuntimeConfig_ECS::GetMinimumUploadBandwidthBps() const
{
    return m_minimumUploadBandwidthBps;
}

unsigned RuntimeConfig_ECS::GetMaximumUploadSizeBytes() const
{
    return m_maximumUploadSizeBytes;
}

void RuntimeConfig_ECS::SetEventPriority(std::string const& tenantId, std::string const& eventName, EventPriority priority)
{
    PAL::ScopedMutexLock guard(m_lock);
    m_apiPriorityMap[{tenantId, eventName}] = priority;
}

void RuntimeConfig_ECS::OnEvent(CallbackEventType type, CallBackContext* context)
{
    if (type == CONFIG_UPDATED) {
        updateEcsConfiguration(context->currentConfig);
    }
}

EventPriority RuntimeConfig_ECS::getPriorityFromMap(EventPriorityMap const& map, EventKey const& key)
{
    EventKey defaultEventKey{"", ""};
    EventKey tenantLevelKey{key.tenantId, ""};

    EventPriorityMap::const_iterator i = map.find(key);
    if (i == map.end()) {
        i = map.find(tenantLevelKey);
        if (i == map.end()) {
            i = map.find(defaultEventKey);
        }
    }

    return (i != map.end()) ? i->second : EventPriority_Unspecified;
}

EventPriority RuntimeConfig_ECS::intToPriority(int value)
{
    if (value < EventPriority_Unspecified) {
        value = EventPriority_Off;
    } else if (value >= EventPriority_Immediate) {
        value = EventPriority_Immediate;
    }
    return static_cast<EventPriority>(value);
}

void RuntimeConfig_ECS::updateEcsConfiguration(ecsclient::IEcsConfigPtr const& config)
{
    std::string agent("SCT");
    ARIASDK_LOG_DETAIL("New ECS configuration: %s", config->GetSetting(agent, "", "").c_str());

    // ECS experiment IDs - data arrives divided per ECS project
    // but backward compatibility dictates to use only event names.
    std::map<std::string, std::string> experimentIdsMap;
    for (std::string const& project : config->GetKeys("EventToConfigIdsMapping", "")) {
        for (std::string const& event : config->GetKeys("EventToConfigIdsMapping", project)) {
            experimentIdsMap[event] = config->GetSettingAsString("EventToConfigIdsMapping", project + "/" + event, std::string());
        }
    }

    // Priorities
    EventPriorityMap eventPriorityMap;
    for (std::string const& tenant : config->GetKeys(agent, "Priority")) {
        std::string tenantPath = "Priority/" + tenant;

        EventPriority tenantLevelPriority = intToPriority(
            config->GetSettingAsInt(agent, tenantPath + "/Default", EventPriority_Unspecified));
        eventPriorityMap[{tenant, ""}] = tenantLevelPriority;

        if (tenantLevelPriority != EventPriority_Off) {
            std::string eventsPath = tenantPath + "/Events";

            for (auto const& eventName : config->GetKeys(agent, eventsPath)) {
                eventPriorityMap[{tenant, eventName}] = intToPriority(
                    config->GetSettingAsInt(agent, eventsPath + "/" + eventName, EventPriority_Unspecified));
            }
        }
    }

    {
        PAL::ScopedMutexLock guard(m_lock);

        m_ecsPriorityMap.swap(eventPriorityMap);
        m_experimentIdsMap.swap(experimentIdsMap);

        m_etag                            = config->GetETag();
        m_collectorUrl                    = config->GetSettingAsString(agent, "TelemetryCollectorUrl",                m_defaultConfig->GetCollectorUrl());
        m_metaStatsTenantToken            = config->GetSettingAsString(agent, "MetaStatsTenantToken",                 m_defaultConfig->GetMetaStatsTenantToken());
        m_uploadRetryBackoffConfig        = config->GetSettingAsString(agent, "UploadRetryBackoffConfig",             m_defaultConfig->GetUploadRetryBackoffConfig());
        int value                         = config->GetSettingAsInt(agent,    "SendFrequency/act_stats",              m_defaultConfig->GetMetaStatsSendIntervalSec());
        m_metaStatsSendIntervalSec        = std::max<int>(0, value);
        value                             = config->GetSettingAsInt(agent,    "OfflineStorageMaxSizeBytes",           m_defaultConfig->GetOfflineStorageMaximumSizeBytes());
        m_offlineStorageMaximumSize       = std::max<int>(0, value);
        value                             = config->GetSettingAsInt(agent,    "OfflineStorageResizeThresholdPercent", m_defaultConfig->GetOfflineStorageResizeThresholdPct());
        m_offlineStorageResizeThreshold   = std::max<int>(0, value);
        value                             = config->GetSettingAsInt(agent,    "RetryMaximumCount",                    m_defaultConfig->GetMaximumRetryCount());
        m_maxRetryCount                   = std::max<int>(0, value);
        m_isHttpRequestCompressionEnabled = config->GetSettingAsBool(agent,   "NeedCompress",                         m_defaultConfig->IsHttpRequestCompressionEnabled());
        value                             = config->GetSettingAsInt(agent,    "MinUploadBandwidthBytesPerSec",        m_defaultConfig->GetMinimumUploadBandwidthBps());
        m_minimumUploadBandwidthBps       = std::max<int>(0, value);
        value                             = config->GetSettingAsInt(agent,    "MaxUploadSizeBytes",                   m_defaultConfig->GetMaximumUploadSizeBytes());
        m_maximumUploadSizeBytes          = std::max<int>(0, value);
    }
}


} ARIASDK_NS_END
