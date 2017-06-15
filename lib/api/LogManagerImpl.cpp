// Copyright (c) Microsoft. All rights reserved.

#include "LogManagerImpl.hpp"
#include "LogManager.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "offline/OfflineStorageHandler.hpp"
#include "system/TelemetrySystem.hpp"
#include "system/UtcTelemetrySystem.hpp"
#include "utils/Utils.hpp"
#include "TransmitProfiles.hpp"
#include "EventProperty.hpp"
#include "pal/UtcHelpers.hpp"

#if ARIASDK_PAL_SKYPE
    #include "bwcontrol/BandwidthController_ResourceManager.hpp"
    #include "config/RuntimeConfig_ECS.hpp"
    #include "http/HttpClient_HttpStack.hpp"
#endif

#if ARIASDK_PAL_WIN32
#ifdef _WINRT_DLL
#include "http/HttpClient_WinRt.hpp"
#else 
#include "http/HttpClient_WinInet.hpp"
#endif
//    #include "http/HttpClient_WinInet.hpp"
#endif

namespace ARIASDK_NS_BEGIN {


#ifdef ANDROID
extern ILogManager* g_jniLogManager;
#endif


ILogManager* ILogManager::Create(LogConfiguration const& configuration)
{
    return new LogManagerImpl(configuration);
}


//---


ARIASDK_LOG_INST_COMPONENT_CLASS(LogManagerImpl, "AriaSDK.LogManager", "Aria telemetry client - LogManager class");

LogManagerImpl::LogManagerImpl(LogConfiguration const& configuration)
  : m_httpClient(configuration.httpClient),
    m_runtimeConfig(configuration.runtimeConfig),
    m_bandwidthController(configuration.bandwidthController),
    m_offlineStorage(nullptr),
    m_system(nullptr)
{
    ARIASDK_LOG_DETAIL("New LogManager instance");

    ARIASDK_NS::PAL::initialize();

    m_context.reset(new ContextFieldsProvider(nullptr));

    m_defaultRuntimeConfig.initialize(configuration);
    if (m_runtimeConfig == nullptr) {
#if ARIASDK_PAL_SKYPE
        if (configuration.skypeEcsClient) {
            ARIASDK_LOG_DETAIL("RuntimeConfig: Skype ECS (provided IEcsClient=%p)", configuration.skypeEcsClient);
            m_ownRuntimeConfig.reset(new RuntimeConfig_ECS(*configuration.skypeEcsClient));
        }
#endif
        m_runtimeConfig = m_ownRuntimeConfig.get();
    }
    else {
        ARIASDK_LOG_DETAIL("RuntimeConfig: External %p", m_runtimeConfig);
    }
    if (m_runtimeConfig == nullptr) {
        ARIASDK_LOG_DETAIL("RuntimeConfig: Default/None");
        m_runtimeConfig = &m_defaultRuntimeConfig;
    }
    else {
        m_runtimeConfig->SetDefaultConfig(m_defaultRuntimeConfig);
    }

    bool isWindowsUtcClientRegistrationEnable = PAL::IsUtcRegistrationEnabledinWindows();
    
    if ((configuration.sdkmode > SdkModeTypes::SdkModeTypes_Aria) && isWindowsUtcClientRegistrationEnable)
    {
        ARIASDK_LOG_DETAIL("Initializing UTC physical layer...");
        m_system = new UtcTelemetrySystem(configuration, *m_runtimeConfig, *m_context);
        m_system->start();
        m_alive = true;
        return;
    }

    if (m_httpClient == nullptr) {
#if ARIASDK_PAL_SKYPE
        ARIASDK_LOG_DETAIL("HttpClient: Skype HTTP Stack (provided IHttpStack=%p)", configuration.skypeHttpStack);
        m_ownHttpClient.reset(new HttpClient_HttpStack(configuration.skypeHttpStack));
#elif ARIASDK_PAL_WIN32
        ARIASDK_LOG_DETAIL("HttpClient: WinInet");
#ifdef _WINRT_DLL
        m_ownHttpClient.reset(new HttpClient_WinRt());
#else 
        m_ownHttpClient.reset(new HttpClient_WinInet());
#endif
#else
        #error The library cannot work without an HTTP client implementation.
#endif
        m_httpClient = m_ownHttpClient.get();
    } else {
        ARIASDK_LOG_DETAIL("HttpClient: External %p", m_httpClient);
    }

  

    if (m_bandwidthController == nullptr) {
#if ARIASDK_PAL_SKYPE
        if (configuration.skypeResourceManager) {
            ARIASDK_LOG_DETAIL("BandwidthController: Skype ResourceManager (provided ResourceManager=%p)", configuration.skypeResourceManager.raw());
            m_ownBandwidthController.reset(new BandwidthController_ResourceManager(configuration.skypeResourceManager));
        }
#endif
        m_bandwidthController = m_ownBandwidthController.get();
    } else {
        ARIASDK_LOG_DETAIL("BandwidthController: External %p", m_bandwidthController);
    }
    if (m_bandwidthController == nullptr) {
        ARIASDK_LOG_DETAIL("BandwidthController: None");
    }

    m_offlineStorage.reset(new OfflineStorageHandler(configuration, *m_runtimeConfig));

    m_system = new TelemetrySystem(configuration, *m_runtimeConfig, *m_offlineStorage, *m_httpClient, *m_context, m_bandwidthController);
    ARIASDK_LOG_DETAIL("Telemetry system created, starting up...");
    if (m_system)
    {
        m_system->start();
    }

    ARIASDK_LOG_INFO("Started up and running");
    m_alive = true;

#ifdef ANDROID
    if (g_jniLogManager == nullptr) {
        g_jniLogManager = this;
    }
#endif
}

LogManagerImpl::~LogManagerImpl()
{
    if (m_alive) {
        FlushAndTeardown();
    }
}

void LogManagerImpl::FlushAndTeardown()
{
    ARIASDK_LOG_INFO("Shutting down...");

    {
        std::lock_guard<std::mutex> lock(m_lock);
        if (m_system)
        {
            m_system->stop();
            ARIASDK_LOG_DETAIL("Telemetry system stopped");
            m_system = nullptr;
        }

        for (auto& record : m_loggers) {
            delete record.second;
        }
        m_loggers.clear();

        m_offlineStorage.reset();

        m_ownBandwidthController.reset();
        m_bandwidthController = nullptr;

        m_ownRuntimeConfig.reset();
        m_runtimeConfig = nullptr;

        m_ownHttpClient.reset();
        m_httpClient = nullptr;

        m_context.reset();
    }

    ARIASDK_LOG_INFO("Shutdown complete");

    ARIASDK_NS::PAL::shutdown();

    m_alive = false;

#ifdef ANDROID
    if (g_jniLogManager == this) {
        g_jniLogManager = nullptr;
    }
#endif
}

void LogManagerImpl::Flush()
{
    ARIASDK_LOG_ERROR("Flush() is not implemented");
}

void LogManagerImpl::UploadNow()
{
    if (m_system)
    {
        m_system->UploadNow();
    }
}

void LogManagerImpl::PauseTransmission()
{
    ARIASDK_LOG_INFO("Pausing transmission, cancelling any outstanding uploads...");
    if (m_system)
    {
        m_system->pauseTransmission();
    }
}

void LogManagerImpl::ResumeTransmission()
{
    ARIASDK_LOG_INFO("Resuming transmission...");
    if (m_system)
    {
        m_system->resumeTransmission();
    }
}

/// <summary>Sets the transmit profile by enum</summary>
/// <param name="profile">Profile enum</param>
void LogManagerImpl::SetTransmitProfile(TransmitProfile profile)
{
    TransmitProfiles::setDefaultProfile(profile);
}

/// <summary>
/// Select one of several predefined transmission profiles.
/// </summary>
/// <param name="profile"></param>
bool LogManagerImpl::SetTransmitProfile(const std::string& profile)
{
    ARIASDK_LOG_INFO("SetTransmitProfile: profile=%s", profile.c_str());
    return TransmitProfiles::setProfile(profile);
}

/// <summary>
/// Load transmit profiles from JSON config
/// </summary>
/// <param name="profiles_json">JSON config (see example above)</param>
/// <returns>true on successful profiles load, false if config is invalid</returns>
bool LogManagerImpl::LoadTransmitProfiles(std::string profiles_json)
{
    ARIASDK_LOG_INFO("LoadTransmitProfiles");
    return TransmitProfiles::load(profiles_json);
}

/// <summary>
/// Reset transmission profiles to default settings
/// </summary>
void LogManagerImpl::ResetTransmitProfiles()
{
    ARIASDK_LOG_INFO("ResetTransmitProfiles");
    TransmitProfiles::reset();
}

const std::string& LogManagerImpl::GetTransmitProfileName()
{
    return TransmitProfiles::getProfile();
};

ISemanticContext& LogManagerImpl::GetSemanticContext()
{
    return *m_context;
}

/// <summary>
/// Set global context field - string
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
void LogManagerImpl::SetContext(std::string const& name, std::string const& value, PiiKind piiKind)
{
    ARIASDK_LOG_DETAIL("SetContext(\"%s\", ..., %u)", name.c_str(), piiKind);
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
}

/// <summary>
/// Set global context field - double
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
void LogManagerImpl::SetContext(const std::string& name, double value, PiiKind piiKind) 
{
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop); 
}

/// <summary>
/// Set global context field - int64
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
void LogManagerImpl::SetContext(const std::string& name, int64_t value, PiiKind piiKind) {
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
}

/// <summary>
/// Set global context field - boolean
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
void LogManagerImpl::SetContext(const std::string& name, bool value, PiiKind piiKind) {
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
}

/// <summary>
/// Set global context field - date/time in .NET ticks
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
void LogManagerImpl::SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind) {
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
}

/// <summary>
/// Set global context field - GUID
/// </summary>
/// <param name="name"></param>
/// <param name="value"></param>
/// <param name="piiKind"></param>
void LogManagerImpl::SetContext(const std::string& name, GUID_t value, PiiKind piiKind) {
    ARIASDK_LOG_INFO("SetContext");
    EventProperty prop(value, piiKind);
    m_context->setCustomField(name, prop);
}


ILogger* LogManagerImpl::GetLogger(std::string const& tenantToken, std::string const& source, std::string const& experimentationProject)
{
    if (m_alive)
    {
        ARIASDK_LOG_DETAIL("GetLogger(tenantId=\"%s\", source=\"%s\", experimentationProject=\"%s\")",
            tenantTokenToId(tenantToken).c_str(), source.c_str(), experimentationProject.c_str());

        std::string normalizedTenantToken = toLower(tenantToken);
        std::string normalizedSource = toLower(source);

        std::lock_guard<std::mutex> lock(m_lock);
        auto& logger = m_loggers[normalizedTenantToken];
        if (!logger) {
            logger = new Logger(normalizedTenantToken, normalizedSource, experimentationProject, *this, m_context.get(), *m_runtimeConfig);
        }
        return logger;
    }
    return nullptr;
}

void LogManagerImpl::addIncomingEvent(IncomingEventContextPtr const& event)
{
    if (m_system)
    {
        m_system->addIncomingEventSystem(event);
    }
}

}}} // namespace Microsoft::Applications::Telemetry
