// Copyright (c) Microsoft. All rights reserved.
#ifndef MAT_LOGMANAGER_HPP
#define MAT_LOGMANAGER_HPP

#include <exception>
#include "CommonFields.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4459 4100 4121 4068)
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"

#ifdef _MANAGED
#include <msclr/lock.h>
public ref class LogManagerLock {
public:
    static Object ^ lock = gcnew Object();
};
#else
#include <mutex>
#endif

#include "LogManagerProvider.hpp"

#define LM_SAFE_CALL(method , ... )                 \
    {                                               \
        LM_LOCKGUARD(stateLock());                  \
        if (nullptr != instance)                    \
        {                                           \
            instance-> method ( __VA_ARGS__);       \
            return STATUS_SUCCESS;                  \
        }                                           \
        return STATUS_EFAIL;                        \
    }

#define LM_SAFE_CALL_STR(method , ... )             \
    {                                               \
        LM_LOCKGUARD(stateLock());                  \
        if (nullptr != instance)                    \
        {                                           \
            return instance-> method ( __VA_ARGS__);\
        }                                           \
        return "";                                  \
    }

#define LM_SAFE_CALL_PTR(method , ... )             \
    {                                               \
        LM_LOCKGUARD(stateLock());                  \
        if (nullptr != instance)                    \
        {                                           \
            return instance-> method ( __VA_ARGS__);\
        }                                           \
        return nullptr;                             \
    }

#define LM_SAFE_CALL_PTRREF(method , ... )          \
    {                                               \
        LM_LOCKGUARD(stateLock());                  \
        if (nullptr != instance)                    \
        {                                           \
            return &(instance-> method ( __VA_ARGS__));\
        }                                           \
        return nullptr;                             \
    }

#define LM_SAFE_CALL_VOID(method , ... )            \
    {                                               \
        LM_LOCKGUARD(stateLock());                  \
        if (nullptr != instance)                    \
        {                                           \
            instance-> method ( __VA_ARGS__);       \
            return;                                 \
        }                                           \
    }

#ifdef _MANAGED
#define LM_LOCKGUARD(macro_mutex) msclr::lock l(LogManagerLock::lock);
#else
#define LM_LOCKGUARD(macro_mutex)                   \
    std::lock_guard<std::mutex> TOKENPASTE2(__guard_, __LINE__) (macro_mutex);
#endif

namespace ARIASDK_NS_BEGIN
{
    class LogManagerNotInitializedException : public std::runtime_error
    {
    public:
        LogManagerNotInitializedException(const char* message) noexcept
            : std::runtime_error(message) { }
    };

    /// <summary>
    /// This configuration flag is populated by SDK to indicate if this singleton instance
    /// is running in "host" mode and all LogController methods should be accessible to the
    /// caller.
    /// </summary>
    static constexpr const char * HOST_MODE = "hostMode";

    /// <summary>
    /// This class is used to manage the Events  logging system
    /// </summary>
    template <class ModuleConfiguration> class LogManagerBase
    {
        static_assert(std::is_base_of<ILogConfiguration, ModuleConfiguration>::value, "ModuleConfiguration must derive from LogConfiguration");

    public:
        using basetype = LogManagerBase<ModuleConfiguration>;

    protected:

        /// <summary>
        /// LogManagerBase constructor
        /// </summary>
        LogManagerBase() {};

        /// <summary>
        /// LogManager copy constructor
        /// </summary>
        LogManagerBase(const LogManagerBase&) {};

        /// <summary>
        /// [not implemented] LogManager assignment operator
        /// </summary>
        LogManagerBase& operator=(const LogManagerBase&) {};

        /// <summary>
        /// LogManager destructor
        /// </summary>
        virtual ~LogManagerBase() {};

#ifndef _MANAGED
        /// <summary>
        /// Native code lock used for executing singleton state-management methods in a thread-safe manner.
        /// Managed code uses a different LogManagerLock.
        /// </summary>
        static std::mutex& stateLock()
        {
            // Magic static is thread-safe in C++
            static std::mutex lock;
            return lock;
        }
#endif

        static inline bool isHost()
        {
            return GetLogConfiguration()[HOST_MODE];
        }

        /// <summary>
        /// Concrete instance for servicing all singleton calls
        /// </summary>
        static ILogManager*         instance;

        /// <summary>
        /// Debug event source associated with this singleton
        /// </summary>
        static DebugEventSource& GetDebugEventSource()
        {
            static DebugEventSource debugEventSource;
            return debugEventSource;
        }

        static const char* GetPrimaryToken()
        {
            ILogConfiguration& config = GetLogConfiguration();
            return (const char *)(config[CFG_STR_PRIMARY_TOKEN]);
        }

    public:

        /// <summary>
        /// Returns this module LogConfiguration
        /// </summary>
        static ILogConfiguration& GetLogConfiguration()
        {
            static ModuleConfiguration currentConfig;
            return currentConfig;
        }

        /// <summary>
        /// Initializes the telemetry logging system with default configuraiton and HTTPClient.
        /// </summary>
        /// <returns>A logger instance instantiated with the default tenantToken.</returns>
        static ILogger* Initialize()
        {
           return Initialize(std::string {});
        }

        /// <summary>
        /// Initializes the telemetry logging system with the specified tenantToken.
        /// </summary>
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <returns>A logger instance instantiated with the tenantToken.</returns>
        inline static ILogger* Initialize(const std::string& tenantToken)
        {
           return Initialize(tenantToken, GetLogConfiguration());
        }

        /// <summary>
        /// Initializes the telemetry logging system with the specified ILogConfiguration.
        /// </summary>
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <param name="configuration">ILogConfiguration to be used.</param>
        /// <returns>A logger instance instantiated with the tenantToken.</returns>
        inline static ILogger* Initialize(const std::string& tenantToken, ILogConfiguration& configuration)
        {
           return Initialize(tenantToken, configuration, nullptr, nullptr);
        }

        /// <summary>
        /// Initializes the telemetry logging system with the specified ILogConfiguration.
        /// </summary>
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <param name="httpClient">IHttpClient implementation to be used, nullptr uses the default</param>
        /// <returns>A logger instance instantiated with the tenantToken.</returns>
        inline static ILogger* Initialize(const std::string& tenantToken, IHttpClient* httpClient)
        {
           return Initialize(tenantToken, GetLogConfiguration(), httpClient, nullptr);
        }

        /// <summary>
        /// Initializes the telemetry logging system with the specified ILogConfiguration.
        /// </summary>
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <param name="httpClient">IHttpClient implementation to be used, nullptr uses the default</param>
        /// <param name="dataViewer">IDataViewer implementation to be registered at the very start, nullptr doesn't register any viewer</param>
        /// <returns>A logger instance instantiated with the tenantToken.</returns>
        inline static ILogger* Initialize(const std::string& tenantToken, IHttpClient* httpClient, const std::shared_ptr<IDataViewer>& dataViewer)
        {
           return Initialize(tenantToken, GetLogConfiguration(), httpClient, dataViewer);
        }

        /// <summary>
        /// Initializes the telemetry logging system with the specified ILogConfiguration and IHttpClient.
        /// </summary>
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <param name="configuration">ILogConfiguration to be used.</param>
        /// <param name="httpClient">IHttpClient implementation to be used, nullptr uses the default</param>
        /// <param name="dataViewer">IDataViewer implementation to be registered at the very start, nullptr doesn't register any viewer</param>
        /// <returns>A logger instance instantiated with the tenantToken.</returns>
        static ILogger* Initialize(
            const std::string& tenantToken,
            ILogConfiguration& configuration,
            IHttpClient* httpClient,
            const std::shared_ptr<IDataViewer>& dataViewer
        )
        {
            LM_LOCKGUARD(stateLock());
            ILogConfiguration& currentConfig = GetLogConfiguration();
            if (nullptr == instance)
            {
                // Copy alternate configuration into currentConfig
                if (&configuration != &currentConfig)
                {
                    for (const auto &kv : configuration)
                    {
                        currentConfig[kv.first] = kv.second;
                    }
                }

                // Assume that if token isn't provided, then it's part of config
                if (!tenantToken.empty())
                {
                    currentConfig[CFG_STR_PRIMARY_TOKEN] = tenantToken;
                }

                status_t status = STATUS_SUCCESS;
                instance = LogManagerProvider::CreateLogManager(currentConfig, httpClient, dataViewer, status);
                instance->AttachEventSource(GetDebugEventSource());
                return instance->GetLogger(currentConfig[CFG_STR_PRIMARY_TOKEN]);
            }
            else {
                // TODO: [MG] - decide what to do if someone's doing re-Initialize.
                // For now Re-initialize works as GetLogger with an alternate token.
                // We may decide to assert(tenantToken==primaryToken) ...
                //
                // If assertion fails, it means someone tries to re-Initialize with
                // different settings and it creates ambiguity for the GetLogger()
                // API call.
            }
            return instance->GetLogger(tenantToken);
        }

        /// <summary>
        /// Flush any pending telemetry events in memory to disk and tear down the telemetry logging system.
        /// </summary>
        static status_t FlushAndTeardown()
        {
#ifdef NO_TEARDOWN // Avoid destroying our ILogManager instance on teardown
            LM_SAFE_CALL(Flush);
            LM_SAFE_CALL(UploadNow);
            return STATUS_SUCCESS;
#else       // Less safe approach, but this is in alignment with original v1 behavior
            // Side-effect of this is that all ILogger instances get invalidated on FlushAndTeardown
            auto callFTD = []() LM_SAFE_CALL(GetLogController()->FlushAndTeardown);
            auto result = callFTD();
            if (instance)
            {
                ILogConfiguration& currentConfig = GetLogConfiguration();
                result = LogManagerProvider::Release(currentConfig);
                instance = nullptr;
            }
            return result;
#endif
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // SDK_GUEST_MODE is a special flag that enforces all the restrictions of a Guest API set.   //
        // Library module that has no control of event flow functionality should define this at      //
        // compile time. If there is a piece of code logic that requires the LogController API set,  //
        // then the module should call GetController() method, which will return an instance that    //
        // allows to control all the aspects of the log flow.                                        //
        ///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SDK_GUEST_MODE

        /// <summary>
        /// Try to send any pending telemetry events in memory or on disk.
        /// </summary>
        static status_t UploadNow()
        {
            if (isHost())
                LM_SAFE_CALL(GetLogController()->UploadNow);
            return STATUS_EPERM; // Permission denied
        }

        /// <summary>
        /// Flush any pending telemetry events in memory to disk to reduce possible data loss as seen necessary.
        /// This function can be very expensive so should be used sparingly. OS will block the calling thread 
        /// and might flush the global file buffers, i.e. all buffered filesystem data, to disk, which could be
        /// time consuming.
        /// </summary>
        static status_t Flush()
        {
            if (isHost())
                LM_SAFE_CALL(GetLogController()->Flush);
            return STATUS_EPERM; // Permission denied
        }

        /// <summary>
        /// Pauses the transmission of events to data collector.
        /// While pasued events will continue to be queued up on client side in cache (either in memory or on disk file).
        /// </summary>
        static status_t PauseTransmission()
        {
            if (isHost())
                LM_SAFE_CALL(GetLogController()->PauseTransmission);
            return STATUS_EPERM; // Permission denied
        }

        /// <summary>
        /// Resumes the transmission of events to data collector.
        /// </summary>
        static status_t ResumeTransmission()
        {
            if (isHost())
                LM_SAFE_CALL(GetLogController()->ResumeTransmission);
            return STATUS_EPERM; // Permission denied
        }

        /// <summary>
        /// Sets transmit profile for event transmission to one of the built-in profiles.
        /// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
        /// based on which to determine how events are to be transmitted. 
        /// </summary>
        /// <param name="profile">Transmit profile</param>
        /// <returns>This function doesn't return any value because it always succeeds.</returns>
        static status_t SetTransmitProfile(TransmitProfile profile)
        {
            if (isHost())
                LM_SAFE_CALL(GetLogController()->SetTransmitProfile, profile);
            return STATUS_EPERM; // Permission denied
        }

        /// <summary>
        /// Sets transmit profile for event transmission.
        /// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
        /// based on which to determine how events are to be transmitted. 
        /// </summary>
        /// <param name="profile">Transmit profile</param>
        /// <returns>true if profile is successfully applied, false otherwise</returns>
        static status_t SetTransmitProfile(const std::string& profile)
        {
            if (isHost())
                LM_SAFE_CALL(GetLogController()->SetTransmitProfile, profile);
            return STATUS_EPERM; // Permission denied
        }

        /// <summary>
        /// Load transmit profiles from JSON config
        /// </summary>
        /// <param name="profiles_json">JSON config (see example above)</param>
        /// <returns>true on successful profiles load, false if config is invalid</returns>
        static status_t LoadTransmitProfiles(const std::string& profiles_json)
        {
            if (isHost())
                LM_SAFE_CALL(GetLogController()->LoadTransmitProfiles, profiles_json);
            return STATUS_EPERM; // Permission denied
        }

        /// <summary>
        /// Reset transmission profiles to default settings
        /// </summary>
        static status_t ResetTransmitProfiles()
        {
            if (isHost())
                LM_SAFE_CALL(GetLogController()->ResetTransmitProfiles);
            return STATUS_EPERM; // Permission denied
        }

#endif

        /// <summary>Get profile name based on built-in profile enum<summary>
        /// <param name="profile">Transmit profile</param>
        static std::string GetTransmitProfileName()
            LM_SAFE_CALL_STR(GetTransmitProfileName);

        /// <summary>
        /// Retrieve an ISemanticContext interface through which to specify context information 
        /// such as device, system, hardware and user information.
        /// Context information set via this API will apply to all logger instance unless they 
        /// are overwritten by individual logger instance.
        /// </summary>
        /// <returns>ISemanticContext interface pointer</returns>
        static ISemanticContext* GetSemanticContext()
            LM_SAFE_CALL_PTRREF(GetSemanticContext);

        /// <summary>
        /// Adds or overrides a property of the custom context for the telemetry logging system.
        /// Context information set here applies to events generated by all ILogger instances 
        /// unless it is overwritten on a particular ILogger instance.
        /// </summary>
        /// <param name="name">Name of the context property</param>
        /// <param name="value">Value of the context property</param>
        /// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
        static status_t SetContext(const std::string& name, const std::string& value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the custom context for the telemetry logging system.
        /// Context information set here applies to events generated by all ILogger instances 
        /// unless it is overwritten on a particular ILogger instance.
        /// </summary>
        /// <param name="name">Name of the context property</param>
        /// <param name="value">Value of the context property</param>
        /// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
        static status_t SetContext(const std::string& name, const char *value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, std::string(value), piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">Double value of the property</param>
        static status_t SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">64-bit Integer value of the property</param>
        static status_t SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">8-bit Integer value of the property</param>
        static status_t SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, (int64_t)value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">16-bit Integer value of the property</param>
        static status_t SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, (int64_t)value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">32-bit Integer value of the property</param>
        static status_t SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, (int64_t)value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">8-bit unsigned integer value of the property</param>
        static status_t SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, (int64_t)value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">16-bit unsigned integer value of the property</param>
        static status_t SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, (int64_t)value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">32-bit unsigned integer value of the property</param>
        static status_t SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, (int64_t)value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">64-bit unsigned integer value of the property</param>
        static status_t SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, (int64_t)value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">Boolean value of the property</param>
        static status_t SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">.NET time ticks</param>
        static status_t SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, value, piiKind);

        /// <summary>
        /// Adds or overrides a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">GUID</param>
        static status_t SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None)
            LM_SAFE_CALL(SetContext, name, value, piiKind);

        /// <summary>
        /// Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
        /// </summary>
        /// <returns>Pointer to the Ilogger interface of an logger instance</returns>
        static ILogger* GetLogger()
            LM_SAFE_CALL_PTR(GetLogger, GetPrimaryToken());

        /// <summary>
        /// Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
        /// </summary>
        /// <param name="source">Source name of events sent by this logger instance</param>
        /// <returns>Pointer to the Ilogger interface of the logger instance</returns>
        static ILogger* GetLogger(const std::string& source)
            LM_SAFE_CALL_PTR(GetLogger, GetPrimaryToken(), source);

        /// <summary>
        /// Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
        /// </summary>
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <param name="source">Source name of events sent by this logger instance</param>
        /// <returns>Pointer to the Ilogger interface of the logger instance</returns>
        static ILogger* GetLogger(const std::string& tenantToken, const std::string& source)
            LM_SAFE_CALL_PTR(GetLogger, tenantToken, source);

        /// <summary>
        /// Get Auth token controller
        /// </summary>
        static IAuthTokensController* GetAuthTokensController()
            LM_SAFE_CALL_PTR(GetAuthTokensController);

        inline static IEventFilterCollection& GetEventFilters()
        {
            LM_LOCKGUARD(stateLock());
            if (nullptr != instance)
            {
                return instance->GetEventFilters();
            }
            throw LogManagerNotInitializedException("LogManager::Initialize must be invoked prior to calling GetFilters()");
        }

        /// <summary>
        /// Add Debug callback
        /// </summary>
        static void AddEventListener(DebugEventType type, DebugEventListener &listener)
        {
            GetDebugEventSource().AddEventListener(type, listener);
        }

        /// <summary>
        /// Remove Debug callback
        /// </summary>
        static void RemoveEventListener(DebugEventType type, DebugEventListener &listener)
        {
            GetDebugEventSource().RemoveEventListener(type, listener);
        }

        /// <summary>
        /// Dispatches a debug event of the specified type.
        /// </summary>
        /// <param name="type">One of the DebugEventType enumeration types.</param>
        static bool DispatchEvent(DebugEventType type)
        {
            return GetDebugEventSource().DispatchEvent(type);
        }

        /// <summary>
        /// Dispatches the specified event to a client callback.
        /// </summary>
        /// <param name="evt">A reference to a DebugEvent object.</param>
        static bool DispatchEvent(DebugEvent evt)
        {
            return GetDebugEventSource().DispatchEvent(std::move(evt));
        }

        /// <summary>
        /// Gets the log session data.
        /// </summary>
        /// <returns>The log session data in a pointer to a LogSessionData object.</returns>
        static LogSessionData* GetLogSessionData()
            LM_SAFE_CALL_PTR(GetLogSessionData);

        /// <summary>
        /// Sets the diagnostic level filter for the LogManager
        /// </summary>
        /// <param name="defaultLevel">Diagnostic level for the LogManager</param>
        /// <param name="levelMin">Minimum level to be sent</param>
        /// <param name="levelMin">Maximum level to be sent</param>
        static void SetLevelFilter(uint8_t defaultLevel, uint8_t levelMin, uint8_t levelMax)
            LM_SAFE_CALL_VOID(SetLevelFilter, defaultLevel, levelMin, levelMax);

        /// <summary>
        /// Sets the diagnostic level filter for the LogManager
        /// </summary>
        /// <param name="defaultLevel">Diagnostic level for the LogManager</param>
        /// <param name="allowedLevels">Set with levels that are allowed to be sent</param>
        static void SetLevelFilter(uint8_t defaultLevel, const std::set<uint8_t>& allowedLevels)
            LM_SAFE_CALL_VOID(SetLevelFilter, defaultLevel, allowedLevels);

        static ILogController* GetController()
        {
            // No-op LogManager is implemented as C++11 magic local static
            static NullLogManager nullLogManager;
            if (!isHost())
                return nullLogManager.GetLogController();
            return instance->GetLogController();
        }

        /// <summary>
        /// Reconfigure the log manager instance.
        /// Reserved for future use to notify SDK about ILogConfiguration & contents change.
        /// </summary>
        static status_t Configure()
            LM_SAFE_CALL(Configure);

        static IDataViewerCollection& GetDataViewerCollection()
        {
            return instance->GetDataViewerCollection();
        }
    };

    // Implements LogManager<T> singleton template static  members
#if (defined(_MANAGED) || defined(_MSC_VER)) && (!defined(__clang__))
// Definition that is compatible with managed and native code compiled with MSVC.
// Unfortuantey we can't use ISO C++11 template definitions because of compiler bug
// that causes improper global static templated member initialization:
// https://developercommunity.visualstudio.com/content/problem/134886/initialization-of-template-static-variable-wrong.html
//
#define DEFINE_LOGMANAGER(LogManagerClass, LogConfigurationClass)                           \
    ILogManager*            LogManagerClass::instance      = nullptr;
#else
// ISO C++ -compliant declaration
#define DEFINE_LOGMANAGER(LogManagerClass, LogConfigurationClass)                                       \
    template<> ILogManager*            LogManagerBase<LogConfigurationClass>::instance {};
#endif

} ARIASDK_NS_END

#pragma clang diagnostic pop

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
