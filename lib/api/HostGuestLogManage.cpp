// Copyright (c) Microsoft. All rights reserved.
#include "pal/PAL.hpp"
#include "HostGuestLogManager.hpp"
#include "CommonLogManagerInternal.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN {

ARIASDK_LOG_INST_COMPONENT_CLASS(HostGuestLogManager, "AriaSDK.HostGuestLogManager", "Aria telemetry client - HostGuestLogManager class");

HostGuestLogManager::HostGuestLogManager(LogConfiguration* config)
    :m_isCreated(false)
{
    ARIASDK_LOG_DETAIL("New HostGuestLogManager instance");
    {
        std::lock_guard<std::mutex> lock(m_lock);
        if (!CommonLogManagerInternal::IsInitialized())
        {
        }
        CommonLogManagerInternal::Initialize(config);
        m_isCreated = true;
        m_context.reset(new ContextFieldsProvider(nullptr));
    }
}

HostGuestLogManager::~HostGuestLogManager()
{
    ARIASDK_LOG_INFO("destructor");
    {
        std::lock_guard<std::mutex> lock(m_lock);
        if (m_isCreated)
        {
            m_isCreated = false;
            CommonLogManagerInternal::FlushAndTeardown();
        }
        for (auto& record : m_loggers) 
        {
            delete record.second;
        }
        m_loggers.clear();
    }
}

ACTStatus HostGuestLogManager::FlushAndTeardown()
{
    ARIASDK_LOG_INFO("Shutting down...");
    {
        std::lock_guard<std::mutex> lock(m_lock);
        if (m_isCreated)
        {
            m_isCreated = false;
            return CommonLogManagerInternal::FlushAndTeardown();
        }
    }
    return ACTStatus::ACTStatus_Fail;
}

ACTStatus HostGuestLogManager::Flush()
{
    ARIASDK_LOG_ERROR("Flush() is not implemented");
    return ACTStatus::ACTStatus_NotSupported;
}

ACTStatus HostGuestLogManager::UploadNow()
{
    return CommonLogManagerInternal::UploadNow();
}

ACTStatus HostGuestLogManager::PauseTransmission()
{
    ARIASDK_LOG_INFO("Pausing transmission, cancelling any outstanding uploads...");
    return CommonLogManagerInternal::PauseTransmission();
}

ACTStatus HostGuestLogManager::ResumeTransmission()
{
    ARIASDK_LOG_INFO("Resuming transmission...");
    return CommonLogManagerInternal::ResumeTransmission();
}

/// <summary>Sets the transmit profile by enum</summary>
/// <param name="profile">Profile enum</param>
ACTStatus HostGuestLogManager::SetTransmitProfile(TransmitProfile profile)
{
    return CommonLogManagerInternal::SetTransmitProfile(profile);
}

/// <summary>
/// Select one of several predefined transmission profiles.
/// </summary>
/// <param name="profile"></param>
ACTStatus HostGuestLogManager::SetTransmitProfile(const std::string& profile)
{
    ARIASDK_LOG_INFO("SetTransmitProfile: profile=%s", profile.c_str());
    return CommonLogManagerInternal::SetTransmitProfile(profile);
}

/// <summary>
/// Load transmit profiles from JSON config
/// </summary>
/// <param name="profiles_json">JSON config (see example above)</param>
/// <returns>true on successful profiles load, false if config is invalid</returns>
ACTStatus HostGuestLogManager::LoadTransmitProfiles(std::string profiles_json)
{
    ARIASDK_LOG_INFO("LoadTransmitProfiles");
    return CommonLogManagerInternal::LoadTransmitProfiles(profiles_json);
}

/// <summary>
/// Reset transmission profiles to default settings
/// </summary>
ACTStatus HostGuestLogManager::ResetTransmitProfiles()
{
    ARIASDK_LOG_INFO("ResetTransmitProfiles");
    return CommonLogManagerInternal::ResetTransmitProfiles();
}

const std::string& HostGuestLogManager::GetTransmitProfileName()
{
    return CommonLogManagerInternal::GetTransmitProfileName();
};

ISemanticContext& HostGuestLogManager::GetSemanticContext()
{
    return *m_context;
}

/// <summary>
/// Set global context field - string
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
ACTStatus HostGuestLogManager::SetContext(std::string const& name, std::string const& value, PiiKind piiKind)
{
    ARIASDK_LOG_DETAIL("SetContext(\"%s\", ..., %u)", name.c_str(), piiKind);
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
    return ACTStatus::ACTStatus_OK;
}

/// <summary>
/// Set global context field - double
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
ACTStatus HostGuestLogManager::SetContext(const std::string& name, double value, PiiKind piiKind) 
{
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop); 
    return ACTStatus::ACTStatus_OK;
}

/// <summary>
/// Set global context field - int64
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
ACTStatus HostGuestLogManager::SetContext(const std::string& name, int64_t value, PiiKind piiKind) {
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
    return ACTStatus::ACTStatus_OK;
}

/// <summary>
/// Set global context field - boolean
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
ACTStatus HostGuestLogManager::SetContext(const std::string& name, bool value, PiiKind piiKind) {
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
    return ACTStatus::ACTStatus_OK;
}

/// <summary>
/// Set global context field - date/time in .NET ticks
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
ACTStatus HostGuestLogManager::SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind) {
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
    return ACTStatus::ACTStatus_OK;
}

/// <summary>
/// Set global context field - GUID
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
ACTStatus HostGuestLogManager::SetContext(const std::string& name, GUID_t value, PiiKind piiKind) {
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
    return ACTStatus::ACTStatus_OK;
}


ILogger* HostGuestLogManager::GetLogger(std::string const& tenantToken)
{
     ARIASDK_LOG_DETAIL("GetLogger(tenantId=\"%s\" ", tenantTokenToId(tenantToken).c_str());
     {
         std::string normalizedTenantToken = toLower(tenantToken);
         std::lock_guard<std::mutex> lock(m_lock);         

          auto& logger = m_loggers[normalizedTenantToken];
          if (!logger)
          {
              logger = new Logger(normalizedTenantToken, "", "", nullptr, m_context.get(), nullptr);
          }
          
          return logger;
     }
}

/// <summary>
/// Add Debug callback
/// </summary>
void HostGuestLogManager::AddEventListener(DebugEventType type, DebugEventListener &listener)
{
    CommonLogManagerInternal::AddEventListener(type, listener);
}

/// <summary>
/// Remove Debug callback
/// </summary>
void HostGuestLogManager::RemoveEventListener(DebugEventType type, DebugEventListener &listener)
{
    CommonLogManagerInternal::RemoveEventListener(type, listener);
}

}}} // namespace Microsoft::Applications::Telemetry
