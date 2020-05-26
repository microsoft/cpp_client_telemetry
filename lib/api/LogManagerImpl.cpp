// Copyright (c) Microsoft. All rights reserved.
#ifdef _MSC_VER
// evntprov.h(838) : warning C4459 : declaration of 'Version' hides global declaration
#pragma warning( disable : 4459 )
#endif
#include "mat/config.h"
#include "LogManagerImpl.hpp"

#include "offline/OfflineStorageHandler.hpp"

#include "system/TelemetrySystem.hpp"

#include "utils/Utils.hpp"
#include "TransmitProfiles.hpp"
#include "EventProperty.hpp"
#include "http/HttpClientFactory.hpp"
#include "pal/TaskDispatcher.hpp"

#ifdef HAVE_MAT_UTC
#if defined __has_include
#  if __has_include ("modules/utc/UtcTelemetrySystem.hpp")
#    include "modules/utc/UtcTelemetrySystem.hpp"
#  else
   /* Compiling without UTC support because UTC private header is unavailable */
#  undef HAVE_MAT_UTC
#  endif
#  else
#include "modules/utc/UtcTelemetrySystem.hpp"
#endif
#endif

#ifdef HAVE_MAT_DEFAULT_FILTER
#if defined __has_include
#  if __has_include ("modules/filter/CompliantByDefaultEventFilterModule.hpp")
#    include "modules/filter/CompliantByDefaultEventFilterModule.hpp"
#  else
   /* Compiling without Filtering support because Filtering private header is unavailable */
#  undef HAVE_MAT_DEFAULT_FILTER
#  endif
#  else
#include "modules/filter/LevelChececkingEventFilter.hpp"
#endif
#endif // HAVE_MAT_DEFAULT_FILTER

namespace ARIASDK_NS_BEGIN
{

    bool ILogManager::DispatchEventBroadcast(DebugEvent evt)
    {
        // LOCKGUARD(ILogManagerInternal::managers_lock);
        for (auto instance : ILogManagerInternal::managers)
        {
            instance->DispatchEvent(evt);
        }
        return true;
    }

    MATSDK_LOG_INST_COMPONENT_CLASS(LogManagerImpl, "EventsSDK.LogManager", "Microsoft Telemetry Client - LogManager class");

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
       : LogManagerImpl(configuration, false /*deferSystemStart*/)
    {
    }

    LogManagerImpl::LogManagerImpl(ILogConfiguration& configuration, bool deferSystemStart)
        : m_logConfiguration(configuration),
        m_bandwidthController(nullptr),
        m_offlineStorage(nullptr)
    {
        m_httpClient = std::static_pointer_cast<IHttpClient>(configuration.GetModule(CFG_MODULE_HTTP_CLIENT));
        m_taskDispatcher = std::static_pointer_cast<ITaskDispatcher>(configuration.GetModule(CFG_MODULE_TASK_DISPATCHER));
        m_dataViewer = std::static_pointer_cast<IDataViewer>(configuration.GetModule(CFG_MODULE_DATA_VIEWER));
        m_config = std::unique_ptr<IRuntimeConfig>(new RuntimeConfig_Default(m_logConfiguration));
        setLogLevel(configuration);
        LOG_TRACE("New LogManager instance");

        PAL::initialize(*m_config);
        PAL::registerSemanticContext(&m_context);

        std::string cacheFilePath = MAT::GetAppLocalTempDirectory();
        if ( !m_logConfiguration.HasConfig(CFG_STR_CACHE_FILE_PATH) ||
            (const char *)(m_logConfiguration[CFG_STR_CACHE_FILE_PATH]) == nullptr)
        {
            if (m_logConfiguration.HasConfig(CFG_STR_PRIMARY_TOKEN))
            {
                std::string tenantId = (const char *)m_logConfiguration[CFG_STR_PRIMARY_TOKEN];
                tenantId = MAT::tenantTokenToId(tenantId);
                if ((!cacheFilePath.empty()) && (cacheFilePath.back() != PATH_SEPARATOR_CHAR))
                {
                    // append path separator if required 
                    cacheFilePath += PATH_SEPARATOR_CHAR;
                }
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

        if (m_logConfiguration.HasConfig(CFG_STR_TRANSMIT_PROFILES))
        {
            std::string transmitProfiles = m_logConfiguration[CFG_STR_TRANSMIT_PROFILES];
            if (!transmitProfiles.empty())
            {
                LOG_INFO("Loading custom transmit profiles...");
                LoadTransmitProfiles(transmitProfiles);
            }
        }

        if (m_logConfiguration.HasConfig(CFG_STR_START_PROFILE_NAME))
        {
            std::string transmitProfile = m_logConfiguration[CFG_STR_START_PROFILE_NAME];
            if (!transmitProfile.empty())
            {
                LOG_INFO("Setting custom transmit profile %s", transmitProfile.c_str());
                SetTransmitProfile(transmitProfile);
            }
        }

        // TODO: [MG] - LogSessionData must utilize sqlite3 DB interface instead of filesystem
        m_logSessionData.reset(new LogSessionData(cacheFilePath));

        m_context.SetCommonField(SESSION_ID_LEGACY, PAL::generateUuidString());

        if (m_dataViewer != nullptr)
        {
            m_dataViewerCollection.RegisterViewer(m_dataViewer);
        } else 
        {
            // TODO: [MG] - register default data viewer implementation if enabled?
        }

        if (m_taskDispatcher == nullptr)
        {
            m_taskDispatcher = PAL::getDefaultTaskDispatcher();
        }
        else
        {
            LOG_TRACE("TaskDispatcher: External %p", m_taskDispatcher.get());
        }

#ifdef HAVE_MAT_UTC
        // UTC is not active
        configuration[CFG_STR_UTC][CFG_BOOL_UTC_ACTIVE] = false;

        // UTC functionality is only available on Windows 10 RS2+
        bool isWindowsUtcClientRegistrationEnable = PAL::IsUtcRegistrationEnabledinWindows();
        configuration[CFG_STR_UTC][CFG_BOOL_UTC_ENABLED] = isWindowsUtcClientRegistrationEnable;

        int32_t sdkMode = configuration[CFG_INT_SDK_MODE];
        if ((sdkMode > SdkModeTypes::SdkModeTypes_CS) && isWindowsUtcClientRegistrationEnable)
        {
            // UTC is active
            configuration[CFG_STR_UTC][CFG_BOOL_UTC_ACTIVE] = true;
            LOG_TRACE("Initializing UTC physical layer...");
            m_system.reset(new UtcTelemetrySystem(*this, *m_config, *m_taskDispatcher));
            if (!deferSystemStart)
            {
               m_system->start();
               m_isSystemStarted = true;
            }
            m_alive = true;
            LOG_INFO("Started up and running in UTC mode");
            return;
        }
#endif

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
        if (m_httpClient == nullptr)
        {
            m_httpClient = HttpClientFactory::Create();
#ifdef HAVE_MAT_WININET_HTTP_CLIENT
            HttpClient_WinInet* client = static_cast<HttpClient_WinInet *>(m_httpClient.get());
            if (client != nullptr)
            {
                client->SetMsRootCheck(m_logConfiguration["http"]["msRootCheck"]);
            }
#endif
        }
        else
        {
            LOG_TRACE("HttpClient: External %p", m_httpClient.get());
        }
#else
        if (m_httpClient == nullptr)
        {
           LOG_ERROR("No valid IHTTPClient passed to LogManagerImpl's ILogConfiguration.");
           MATSDK_THROW(std::invalid_argument("configuration"));
        }
#endif

        if (m_bandwidthController == nullptr) {
            m_bandwidthController = m_ownBandwidthController.get();
        }
        else
        {
            LOG_TRACE("BandwidthController: External %p", m_bandwidthController);
        }
        if (m_bandwidthController == nullptr)
        {
            LOG_TRACE("BandwidthController: None");
        }

        m_offlineStorage.reset(new OfflineStorageHandler(*this, *m_config, *m_taskDispatcher));

        m_system.reset(new TelemetrySystem(*this, *m_config, *m_offlineStorage, *m_httpClient, *m_taskDispatcher, m_bandwidthController));
        LOG_TRACE("Telemetry system created, starting up...");
        if (m_system && !deferSystemStart)
        {
            m_system->start();
            m_isSystemStarted = true;
        }

#ifdef HAVE_MAT_DEFAULT_FILTER
        m_modules.push_back(std::unique_ptr<CompliantByDefaultEventFilterModule>(new CompliantByDefaultEventFilterModule()));
#endif // HAVE_MAT_DEFAULT_FILTER

        LOG_INFO("Initializing Modules");
        InitializeModules();

        LOG_INFO("Started up and running");
        m_alive = true;

    }
    
    /// <summary>
    /// Reconfigure this ILogManager instance using current snapshot of ILogConfiguration
    /// </summary>
    void LogManagerImpl::Configure()
    {
        // TODO: [maxgolov] - add other config params.
#ifdef HAVE_MAT_WININET_HTTP_CLIENT
        HttpClient_WinInet* client = static_cast<HttpClient_WinInet *>(m_httpClient.get());
        if (client != nullptr)
        {
            client->SetMsRootCheck(m_logConfiguration["http"]["msRootCheck"]);
        }
#endif
    };


    LogManagerImpl::~LogManagerImpl() noexcept
    {
        FlushAndTeardown();
        LOCKGUARD(ILogManagerInternal::managers_lock);
        ILogManagerInternal::managers.erase(this);
    }

    void LogManagerImpl::FlushAndTeardown()
    {
        LOG_INFO("Shutting down...");
        LOCKGUARD(m_lock);
        if (m_alive)
        {
            LOG_INFO("Tearing down modules");
            TeardownModules();

            if (m_isSystemStarted && m_system)
            {
                m_system->stop();
                LOG_TRACE("Telemetry system stopped");
            }
            m_system = nullptr;

            m_loggers.clear();

            m_offlineStorage.reset();
            m_ownBandwidthController.reset();
            m_bandwidthController = nullptr;

            m_httpClient = nullptr;
            m_taskDispatcher = nullptr;
            m_dataViewer = nullptr;

            m_filters.UnregisterAllFilters();

            auto shutTime = GetUptimeMs();
            PAL::shutdown();
            shutTime = GetUptimeMs() - shutTime;
            LOG_INFO("Shutdown complete in %lld ms", shutTime);
        }
        m_alive = false;

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
        LOCKGUARD(m_lock);
        if (GetSystem())
        {
           GetSystem()->upload();
        }
        // FIXME: [MG] - make sure m_system->upload returns a status
        return STATUS_SUCCESS;
    }

    status_t LogManagerImpl::PauseTransmission()
    {
        LOG_INFO("Pausing transmission, cancelling any outstanding uploads...");
        LOCKGUARD(m_lock);
        if (GetSystem())
        {
           GetSystem()->pause();
        }
        // FIXME: [MG] - make sure m_system->pause returns a status
        return STATUS_SUCCESS;
    }

    status_t LogManagerImpl::ResumeTransmission()
    {
        LOG_INFO("Resuming transmission...");
        LOCKGUARD(m_lock);
        if (GetSystem())
        {
           GetSystem()->resume();
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

    status_t LogManagerImpl::LoadTransmitProfiles(const std::vector<TransmitProfileRules>& profiles) noexcept
    {
        LOG_INFO("LoadTransmitProfiles");
        bool result = TransmitProfiles::load(profiles);
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
        m_context.SetCustomField(name, prop);
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
        m_context.SetCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - int64
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, int64_t value, PiiKind piiKind)
    {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.SetCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - boolean
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, bool value, PiiKind piiKind)
    {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.SetCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - date/time in .NET ticks
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind)
    {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.SetCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Set global context field - GUID
    /// </summary>
    /// <param name="name"></param>
    /// <param name="value"></param>
    /// <param name="piiKind"></param>
    status_t LogManagerImpl::SetContext(const std::string& name, GUID_t value, PiiKind piiKind)
    {
        LOG_INFO("SetContext");
        EventProperty prop(value, piiKind);
        m_context.SetCustomField(name, prop);
        return STATUS_SUCCESS;
    }

    /// <summary>
    /// Obtain current ILogConfiguration instance associated with the LogManager
    /// </summary>
    ILogConfiguration & LogManagerImpl::GetLogConfiguration()
    {
        return m_logConfiguration;
    }


    ILogger* LogManagerImpl::GetLogger(const std::string & tenantToken, const std::string & source, const std::string & scope)
    {
        if (m_alive)
        {
            LOG_TRACE("GetLogger(tenantId=\"%s\", source=\"%s\")", tenantTokenToId(tenantToken).c_str(), source.c_str());

            std::string normalizedTenantToken = toLower(tenantToken);
            std::string normalizedSource = toLower(source);
            std::string hash = normalizedTenantToken + "/" + normalizedSource;

            LOCKGUARD(m_lock);
            auto it = m_loggers.find(hash);
            if (it == std::end(m_loggers))
            {
                m_loggers[hash] = std::unique_ptr<Logger>(new Logger(
                    normalizedTenantToken, normalizedSource, scope,
                    *this, m_context, *m_config));
            }
            uint8_t level = m_diagLevelFilter.GetDefaultLevel();
            if (level != PDL_DEFAULT) 
            {
                m_loggers[hash]->SetPrivacyLevel(level);
            }
            return m_loggers[hash].get();
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
        LOCKGUARD(m_lock);
        if (GetSystem())
        {
           GetSystem()->sendEvent(event);
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

    IEventFilterCollection& LogManagerImpl::GetEventFilters() noexcept
    {
        return m_filters;
    }

    const IEventFilterCollection& LogManagerImpl::GetEventFilters() const noexcept
    {
        return m_filters;
    }

    LogSessionData* LogManagerImpl::GetLogSessionData()
    {
        return m_logSessionData.get();
    }

    void LogManagerImpl::SetLevelFilter(uint8_t defaultLevel, uint8_t levelMin, uint8_t levelMax)
    {
        m_diagLevelFilter.SetFilter(defaultLevel, levelMin, levelMax);
    }

    void LogManagerImpl::SetLevelFilter(uint8_t defaultLevel, const std::set<uint8_t>& allowedLevels)
    {
        m_diagLevelFilter.SetFilter(defaultLevel, allowedLevels);
    }

    const DiagLevelFilter& LogManagerImpl::GetLevelFilter()
    {
        return m_diagLevelFilter;
    }

    std::unique_ptr<ITelemetrySystem>& LogManagerImpl::GetSystem()
    {
        if (m_system == nullptr || m_isSystemStarted)
           return m_system;

        m_system->start();
        m_isSystemStarted = true;
        return m_system;
    }

    void LogManagerImpl::InitializeModules() noexcept
    {
        for (const auto& module : m_modules)
        {
            module->Initialize(this);
        }
    }

    void LogManagerImpl::TeardownModules() noexcept
    {
        for (const auto& module : m_modules)
        {
            module->Teardown();
        }
        std::vector<std::unique_ptr<IModule>>{}.swap(m_modules);
    }

    const IDataViewerCollection& LogManagerImpl::GetDataViewerCollection() const
    {
        return m_dataViewerCollection;
    }

    IDataViewerCollection& LogManagerImpl::GetDataViewerCollection()
    {
        return m_dataViewerCollection;
    }

} ARIASDK_NS_END
