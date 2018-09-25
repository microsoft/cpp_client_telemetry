// Copyright (c) Microsoft. All rights reserved.

#include "LogManagerImpl.hpp"

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
#endif

#if ARIASDK_PAL_CPP11
#include "http/HttpClient.hpp"
#endif

namespace ARIASDK_NS_BEGIN {

#ifdef ANDROID
    extern ILogManager* g_jniLogManager;
#endif

    bool ILogManager::DispatchEventBroadcast(DebugEvent evt)
    {
        // LOCKGUARD(ILogManagerInternal::managers_lock);
        for (auto instance : ILogManagerInternal::managers)
        {
            instance->DispatchEvent(evt);
        }
        return true;
    }

    ARIASDK_LOG_INST_COMPONENT_CLASS(LogManagerImpl, "EventsSDK.LogManager", "Events telemetry client - LogManager class");

#if 1
    // TODO: integrate Tracing API from v1
    // Meanwhile we'd set the g_logLevel using ILogConfiguration settings
    static void setLogLevel(ILogConfiguration& configuration)
    {
        uint32_t minTraceLevel = configuration[CFG_INT_TRACE_LEVEL_MIN];

        switch (minTraceLevel)
        {
        case ACTTraceLevel_Debug:
            PAL::detail::g_logLevel = PAL::LogLevel::Detail;
            break;
        case ACTTraceLevel_Trace:
            PAL::detail::g_logLevel = PAL::LogLevel::Detail;
            break;
        case ACTTraceLevel_Info:
            PAL::detail::g_logLevel = PAL::LogLevel::Info;
            break;
        case ACTTraceLevel_Warn:
            PAL::detail::g_logLevel = PAL::LogLevel::Warning;
            break;
        case ACTTraceLevel_Error:
            PAL::detail::g_logLevel = PAL::LogLevel::Error;
            break;
        case ACTTraceLevel_Fatal:
            PAL::detail::g_logLevel = PAL::LogLevel::Error;
            break;
        default:
            PAL::detail::g_logLevel = PAL::LogLevel::Warning;
            break;
        }
    }
#endif

    LogManagerImpl::LogManagerImpl(ILogConfiguration& configuration)
        : m_httpClient(nullptr),
        m_bandwidthController(nullptr),
        m_offlineStorage(nullptr),
        m_system(nullptr),
        m_logConfiguration(configuration)
    {
        setLogLevel(configuration);
        LOG_TRACE("New LogManager instance");

        PAL::initialize();
        PAL::registerSemanticContext(&m_context);

        std::string cacheFilePath = MAT::GetAppLocalTempDirectory();
        if ( !m_logConfiguration.count(CFG_STR_CACHE_FILE_PATH) ||
            (const char *)(m_logConfiguration[CFG_STR_CACHE_FILE_PATH]) == nullptr)
        {
            if (m_logConfiguration.count(CFG_STR_PRIMARY_TOKEN))
            {
                std::string tenantId = (const char *)m_logConfiguration[CFG_STR_PRIMARY_TOKEN];
                tenantId = MAT::tenantTokenToId(tenantId);

                cacheFilePath += tenantId;
                cacheFilePath += ".db";
            }
            else
            {
                cacheFilePath = ":memory:";
            }
            m_logConfiguration[CFG_STR_CACHE_FILE_PATH] = cacheFilePath;
        }
        else
        {
            std::string filename = (const char *)m_logConfiguration[CFG_STR_CACHE_FILE_PATH];
            if (filename.find(PATH_SEPARATOR_CHAR) == std::string::npos)
            {
                cacheFilePath += filename;
                m_logConfiguration[CFG_STR_CACHE_FILE_PATH] = cacheFilePath;
            }
            // TODO: [MG] - verify that cache file is writeable
        }

        if (m_logConfiguration.count(CFG_STR_TRANSMIT_PROFILES))
        {
            std::string transmitProfiles = m_logConfiguration[CFG_STR_TRANSMIT_PROFILES];
            if (!transmitProfiles.empty())
            {
                LOG_INFO("Loading custom transmit profiles...");
                LoadTransmitProfiles(transmitProfiles);
            }
        }

        if (m_logConfiguration.count(CFG_STR_START_PROFILE_NAME))
        {
            std::string transmitProfile = m_logConfiguration[CFG_STR_START_PROFILE_NAME];
            if (!transmitProfile.empty())
            {
                LOG_INFO("Setting custom transmit profile %s", transmitProfile.c_str());
                SetTransmitProfile(transmitProfile);
            }
        }

        m_config = new RuntimeConfig_Default(m_logConfiguration);

        // TODO: [MG] - LogSessionData must utilize sqlite3 DB interface instead of filesystem
        m_logSessionData.reset(new LogSessionData(cacheFilePath));

        m_context.setCommonField("act_session_id", PAL::generateUuidString()); // GetLogSessionData()->getSessionSDKUid()

#ifdef _WIN32
        // UTC functionality is only available on Windows
        bool isWindowsUtcClientRegistrationEnable = PAL::IsUtcRegistrationEnabledinWindows();

        int32_t sdkMode = configuration[CFG_INT_SDK_MODE];
        if ((sdkMode > SdkModeTypes::SdkModeTypes_Aria) && isWindowsUtcClientRegistrationEnable)
        {
            LOG_TRACE("Initializing UTC physical layer...");
            m_system.reset(new UtcTelemetrySystem(*this, *m_config));
            m_system->start();
            m_alive = true;
            return;
        }
#endif

// ****************************************************************************************** //
// BEGIN TODO: [MG] - simimlar HTTP client initialization code is being used in several spots.
// We should refactor this to abstract away the client implementation type and obtain
// the IHttpClient ptr from one convenient wrapper, maybe naming it "HTTP Client Factory"
        if (m_httpClient == nullptr) {
#if ARIASDK_PAL_SKYPE
            LOG_TRACE("HttpClient: Skype HTTP Stack (provided IHttpStack=%p)", configuration.skypeHttpStack);
            m_ownHttpClient.reset(new HttpClient_HttpStack(configuration.skypeHttpStack));
#elif ARIASDK_PAL_WIN32
            LOG_TRACE("HttpClient: WinInet");
#ifdef _WINRT_DLL
            m_ownHttpClient.reset(new HttpClient_WinRt());
#else 
            m_ownHttpClient.reset(new HttpClient_WinInet());
#endif
#elif ARIASDK_PAL_CPP11
            LOG_TRACE("HttpClient: generic HTTP client");
            m_ownHttpClient.reset(new HttpClient());
#else
#error The library cannot work without an HTTP client implementation.
#endif
            m_httpClient = m_ownHttpClient.get();
        }
        else {
            LOG_TRACE("HttpClient: External %p", m_httpClient);
        }
// END TODO: [MG]
// ****************************************************************************************** //

        if (m_bandwidthController == nullptr) {
#if ARIASDK_PAL_SKYPE
            if (configuration.skypeResourceManager) {
                LOG_TRACE("BandwidthController: Skype ResourceManager (provided ResourceManager=%p)", configuration.skypeResourceManager.raw());
                m_ownBandwidthController.reset(new BandwidthController_ResourceManager(configuration.skypeResourceManager));
            }
#endif
            m_bandwidthController = m_ownBandwidthController.get();
        }
        else {
            LOG_TRACE("BandwidthController: External %p", m_bandwidthController);
        }
        if (m_bandwidthController == nullptr) {
            LOG_TRACE("BandwidthController: None");
        }

        m_offlineStorage.reset(new OfflineStorageHandler(*this, *m_config));

        m_system.reset(new TelemetrySystem(*this, *m_config, *m_offlineStorage, *m_httpClient, m_bandwidthController));
        LOG_TRACE("Telemetry system created, starting up...");
        if (m_system)
        {
            m_system->start();
        }

        LOG_INFO("Started up and running");
        m_alive = true;

#ifdef ANDROID
        if (g_jniLogManager == nullptr) {
            g_jniLogManager = this;
        }
#endif
    }

    void LogManagerImpl::Configure()
    {
    };


    LogManagerImpl::~LogManagerImpl()
    {
        FlushAndTeardown();
        LOCKGUARD(ILogManagerInternal::managers_lock);
        ILogManagerInternal::managers.erase(this);
    }

    void LogManagerImpl::FlushAndTeardown()
    {
        LOG_INFO("Shutting down...");
        {
            LOCKGUARD(m_lock);
            if (m_system)
            {
                m_system->stop();
                LOG_TRACE("Telemetry system stopped");
                m_system.reset();
            }

#if 0
            for (auto& record : m_loggers) {
                delete record.second;
            }
            m_loggers.clear();
#endif

            m_offlineStorage.reset();
            m_ownBandwidthController.reset();
            m_bandwidthController = nullptr;

            m_ownHttpClient.reset();
            m_httpClient = nullptr;

            // Reset the contents of m_eventFilterRegulator, but keep the object
            m_eventFilterRegulator.Reset();
        }

        auto shutTime = GetUptimeMs();
        PAL::shutdown();
        shutTime = GetUptimeMs() - shutTime;
        LOG_INFO("Shutdown complete in %lld ms", shutTime);

        m_alive = false;
#ifdef ANDROID
        if (g_jniLogManager == this) {
            g_jniLogManager = nullptr;
        }
#endif
    }

    status_t LogManagerImpl::Flush()
    {
        LOG_INFO("Flush()");
        if (m_offlineStorage)
            m_offlineStorage->Flush();
        return STATUS_SUCCESS;
    }

    status_t LogManagerImpl::UploadNow()
    {
        if (m_system)
        {
            m_system->upload();
        }
        // FIXME: [MG] - make sure m_system->upload returns a status
        return STATUS_SUCCESS;
    }

    status_t LogManagerImpl::PauseTransmission()
    {
        LOG_INFO("Pausing transmission, cancelling any outstanding uploads...");
        if (m_system)
        {
            m_system->pause();
        }
        // FIXME: [MG] - make sure m_system->pause returns a status
        return STATUS_SUCCESS;
    }

    status_t LogManagerImpl::ResumeTransmission()
    {
        LOG_INFO("Resuming transmission...");
        if (m_system)
        {
            m_system->resume();
        }
        // FIXME: [MG] - make sure m_system->resume returns a status
        return STATUS_SUCCESS;
    }

    /// <summary>Sets the transmit profile by enum</summary>
    /// <param name="profile">Profile enum</param>
    status_t LogManagerImpl::SetTransmitProfile(TransmitProfile profile)
    {
        bool result = TransmitProfiles::setDefaultProfile(profile);
        return (result) ? STATUS_SUCCESS : STATUS_EFAIL;
    }

    /// <summary>
    /// Select one of several predefined transmission profiles.
    /// </summary>
    /// <param name="profile"></param>
    status_t LogManagerImpl::SetTransmitProfile(const std::string& profile)
    {
        LOG_INFO("SetTransmitProfile: profile=%s", profile.c_str());
        bool result = TransmitProfiles::setProfile(profile);
        return (result) ? STATUS_SUCCESS : STATUS_EFAIL;
    }

    /// <summary>
    /// Load transmit profiles from JSON config
    /// </summary>
    /// <param name="profiles_json">JSON config (see example above)</param>
    /// <returns>true on successful profiles load, false if config is invalid</returns>
    status_t LogManagerImpl::LoadTransmitProfiles(const std::string& profiles_json)
    {
        LOG_INFO("LoadTransmitProfiles");
        bool result = TransmitProfiles::load(profiles_json);
        return (result) ? STATUS_SUCCESS : STATUS_EFAIL;
    }

    /// <summary>
    /// Reset transmission profiles to default settings
    /// </summary>
    status_t LogManagerImpl::ResetTransmitProfiles()
    {
        LOG_INFO("ResetTransmitProfiles");
        TransmitProfiles::reset();
        return STATUS_SUCCESS;
    }

    const std::string& LogManagerImpl::GetTransmitProfileName()
    {
        return TransmitProfiles::getProfile();
    };

    ISemanticContext& LogManagerImpl::GetSemanticContext()
    {
        return m_context;
    }

    /// <summary>
    /// Set global context field - string
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(std::string const& name, std::string const& value, PiiKind piiKind)
    {
        LOG_TRACE("SetContext(\"%s\", ..., %u)", name.c_str(), piiKind);
        EventProperty prop(value, piiKind);
        m_context.setCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - double
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, double value, PiiKind piiKind)
    {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.setCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - int64
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, int64_t value, PiiKind piiKind) {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.setCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - boolean
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, bool value, PiiKind piiKind) {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.setCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - date/time in .NET ticks
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind) {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.setCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - GUID
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, GUID_t value, PiiKind piiKind) {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.setCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    ILogger* LogManagerImpl::GetLogger(std::string const& tenantToken, std::string const& source, std::string const& experimentationProject)
    {
        if (m_alive)
        {
            LOG_TRACE("GetLogger(tenantId=\"%s\", source=\"%s\", experimentationProject=\"%s\")",
                tenantTokenToId(tenantToken).c_str(), source.c_str(), experimentationProject.c_str());

            std::string normalizedTenantToken = toLower(tenantToken);
            std::string normalizedSource = toLower(source);
            std::string hash = normalizedTenantToken + "/" + normalizedSource;

            LOCKGUARD(m_lock);
            auto it = m_loggers.find(hash);
            if (it == std::end(m_loggers))
            {
                m_loggers[hash] = new Logger(
                    normalizedTenantToken, normalizedSource, experimentationProject,
                    *this, m_context, *m_config,
                    m_eventFilterRegulator.GetTenantFilter(normalizedTenantToken));
            }
            return m_loggers[hash];
        }
        return nullptr;
    }

    /// <summary>
    /// Adds the event listener.
    /// </summary>
    /// <param name="type">The type.</param>
    /// <param name="listener">The listener.</param>
    void LogManagerImpl::AddEventListener(DebugEventType type, DebugEventListener &listener)
    {
        m_debugEventSource.AddEventListener(type, listener);
    };

    /// <summary>
    /// Removes the event listener.
    /// </summary>
    /// <param name="type">The type.</param>
    /// <param name="listener">The listener.</param>
    void LogManagerImpl::RemoveEventListener(DebugEventType type, DebugEventListener &listener)
    {
        m_debugEventSource.RemoveEventListener(type, listener);
    };

    /// <summary>
    /// Dispatches the event.
    /// </summary>
    /// <param name="evt">The evt.</param>
    /// <returns></returns>
    bool LogManagerImpl::DispatchEvent(DebugEvent evt)
    {
        return m_debugEventSource.DispatchEvent(std::move(evt));
    };

    /// <summary>Attach cascaded DebugEventSource to forward all events to</summary>
    bool LogManagerImpl::AttachEventSource(DebugEventSource & other)
    {
        return m_debugEventSource.AttachEventSource(other);
    }

    /// <summary>Detach cascaded DebugEventSource to forward all events to</summary>
    bool LogManagerImpl::DetachEventSource(DebugEventSource & other)
    {
        return m_debugEventSource.DetachEventSource(other);
    }

    void LogManagerImpl::sendEvent(IncomingEventContextPtr const& event)
    {
        if (m_system)
        {
            m_system->sendEvent(event);
        }
    }

    ILogController* LogManagerImpl::GetLogController()
    {
        return this;
    }

    IAuthTokensController* LogManagerImpl::GetAuthTokensController()
    {
        return &m_authTokensController;
    }

    LogSessionData* LogManagerImpl::GetLogSessionData()
    {
        return m_logSessionData.get();
    }


    status_t LogManagerImpl::SetExclusionFilter(const char* tenantToken, const char** filterStrings, uint32_t filterCount)
    {
        return m_eventFilterRegulator.SetExclusionFilter(tenantToken, filterStrings, filterCount);
    }

    status_t LogManagerImpl::SetExclusionFilter(const char* tenantToken, const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount)
    {
        return m_eventFilterRegulator.SetExclusionFilter(tenantToken, filterStrings, filterRates, filterCount);
    }

} ARIASDK_NS_END
