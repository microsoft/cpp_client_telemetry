// Copyright (c) Microsoft. All rights reserved.
#ifndef LOGMANAGERIMPL_HPP
#define LOGMANAGERIMPL_HPP

#include "LogConfiguration.hpp"
#include "config/RuntimeConfig_Default.hpp"

#include "system/Contexts.hpp"

#include "IDecorator.hpp"
#include "IHttpClient.hpp"
#include "ILogManager.hpp"
#include "IModule.hpp"
#include "ITaskDispatcher.hpp"

#include "api/ContextFieldsProvider.hpp"
#include "api/Logger.hpp"

#include "DebugEvents.hpp"
#include <memory>

#include "IBandwidthController.hpp"
#include "api/AuthTokensController.hpp"
#include "api/DataViewerCollection.hpp"
#include "filter/EventFilterCollection.hpp"

#include "AllowedLevelsCollection.hpp"

#include "offline/LogSessionDataProvider.hpp"

#include <mutex>
#include <set>

namespace MAT_NS_BEGIN
{
    class ITelemetrySystem;

    class DiagLevelFilter final
    {
       public:
        DiagLevelFilter() :
            m_levelMin(DIAG_LEVEL_DEFAULT_MIN),
            m_levelMax(DIAG_LEVEL_DEFAULT_MAX),
            m_level(DIAG_LEVEL_DEFAULT),
            m_levelSet({})
        {
        }

        /// <summary>
        /// Internal method that allows to obtain the default level
        /// </summary>
        uint8_t GetDefaultLevel() const
        {
            return m_level;
        }

        /// <summary>
        /// Verify if logging is enabled for given level
        /// </summary>
        /// <param name="level">Diagnostic level.</param>
        bool IsLevelEnabled(uint8_t level) const
        {
            if (!m_levelSet.empty())
            {
                return m_levelSet.find(level) != m_levelSet.end();
            }
            return m_levelMin <= m_levelMax && m_levelMin <= level && level <= m_levelMax;
        }

        /// <summary>
        /// Method that checks if the filtering has been enabled
        /// </summary>
        bool IsLevelFilterEnabled() const
        {
            return !m_levelSet.empty() || m_levelMin != DIAG_LEVEL_DEFAULT_MIN || m_levelMax != DIAG_LEVEL_DEFAULT_MAX || m_level != DIAG_LEVEL_DEFAULT;
        }

        /// <summary>
        /// Method that allows to set the filter for the LogManager
        /// <param name="defaultLevel">Diag level for the LogManager</param>
        /// <param name="levelMin">Min level to enable</param>
        /// <param name="levelMax">Max level to enable</param>
        /// </summary>
        void SetFilter(uint8_t defaultLevel, uint8_t levelMin, uint8_t levelMax)
        {
            m_level = defaultLevel;
            m_levelMin = levelMin;
            m_levelMax = levelMax;
        }

        /// <summary>
        /// Method that allows to set the filter for the LogManager
        /// <param name="defaultLevel">Diag level for the LogManager</param>
        /// <param name="allowedLevels">Set with the enabled levels</param>
        /// </summary>
        void SetFilter(uint8_t defaultLevel, const std::set<uint8_t>& allowedLevels)
        {
            m_level = defaultLevel;
            m_levelSet = allowedLevels;
        }

       private:
        uint8_t m_levelMin;
        uint8_t m_levelMax;
        uint8_t m_level;
        std::set<uint8_t> m_levelSet;
    };

    class ILogManagerInternal : public ILogManager
    {
       public:
        static std::recursive_mutex managers_lock;
        static std::set<ILogManager*> managers;

        virtual void sendEvent(IncomingEventContextPtr const& event) = 0;
        virtual const ContextFieldsProvider& GetContext() = 0;
        virtual const DiagLevelFilter& GetLevelFilter() = 0;
    };

    class Logger;

    using LoggerMap = std::map<std::string, std::unique_ptr<Logger>>;

    class DeadLoggers
    {
       public:
        void AddMap(LoggerMap&& source);
        size_t GetDeadLoggerCount() const noexcept;

        std::vector<std::unique_ptr<Logger>> m_deadLoggers;
        mutable std::mutex m_deadLoggersMutex;
    };

    class LogManagerImpl : public ILogManagerInternal
    {
       public:
        LogManagerImpl(ILogConfiguration& configuration);
        LogManagerImpl(ILogConfiguration& configuration, bool deferSystemStart);

        virtual ~LogManagerImpl() noexcept override;

        /**
         * ILogController - state management methods
         */
        virtual void Configure() override;

        virtual void FlushAndTeardown() override;

        virtual status_t Flush() override;
        virtual status_t UploadNow() override;
        virtual status_t PauseTransmission() override;
        virtual status_t ResumeTransmission() override;
        virtual status_t SetTransmitProfile(TransmitProfile profile) override;
        virtual status_t SetTransmitProfile(const std::string& profile) override;
        virtual status_t LoadTransmitProfiles(const std::string& profiles_json) override;
        virtual status_t LoadTransmitProfiles(const std::vector<TransmitProfileRules>& profiles) noexcept override;
        virtual status_t ResetTransmitProfiles() override;
        virtual const std::string& GetTransmitProfileName() override;

        /**
         * Semantic Context methods
         */
        virtual ISemanticContext& GetSemanticContext() override;

        virtual status_t SetContext(std::string const& name, std::string const& value, PiiKind piiKind = PiiKind_None) override;

        virtual status_t SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None) override;

        virtual status_t SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None) override;

        virtual inline status_t SetContext(const std::string& name, const char* value, PiiKind piiKind = PiiKind_None) override
        {
            const std::string val(value);
            return SetContext(name, val, piiKind);
        };

        virtual inline status_t SetContext(const std::string& name, int8_t value, PiiKind piiKind = PiiKind_None) override
        {
            return SetContext(name, (int64_t)value, piiKind);
        }

        virtual inline status_t SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) override
        {
            return SetContext(name, (int64_t)value, piiKind);
        }

        virtual inline status_t SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) override
        {
            return SetContext(name, (int64_t)value, piiKind);
        }

        virtual inline status_t SetContext(const std::string& name, uint8_t value, PiiKind piiKind = PiiKind_None) override
        {
            return SetContext(name, (int64_t)value, piiKind);
        }

        virtual inline status_t SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) override
        {
            return SetContext(name, (int64_t)value, piiKind);
        }

        virtual inline status_t SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) override
        {
            return SetContext(name, (int64_t)value, piiKind);
        }

        virtual inline status_t SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) override
        {
            return SetContext(name, (int64_t)value, piiKind);
        }

        virtual status_t SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None) override;

        virtual status_t SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None) override;

        virtual status_t SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None) override;

        virtual ILogConfiguration& GetLogConfiguration() override;

        virtual ILogger* GetLogger(std::string const& tenantToken, std::string const& source = std::string(), std::string const& scopeId = std::string()) override;

        LogSessionData* GetLogSessionData() override;

        ILogController* GetLogController(void) override;

        IAuthTokensController* GetAuthTokensController() override;

        IEventFilterCollection& GetEventFilters() noexcept override;

        const IEventFilterCollection& GetEventFilters() const noexcept override;

        /// <summary>
        /// Adds the event listener.
        /// </summary>
        /// <param name="type">The type.</param>
        /// <param name="listener">The listener.</param>
        virtual void AddEventListener(DebugEventType type, DebugEventListener& listener) override;

        /// <summary>
        /// Removes the event listener.
        /// </summary>
        /// <param name="type">The type.</param>
        /// <param name="listener">The listener.</param>
        virtual void RemoveEventListener(DebugEventType type, DebugEventListener& listener) override;

        /// <summary>
        /// Dispatches the event.
        /// </summary>
        /// <param name="evt">The evt.</param>
        /// <returns></returns>
        virtual bool DispatchEvent(DebugEvent evt) override;

        ///
        virtual bool AttachEventSource(DebugEventSource& other) override;

        ///
        virtual bool DetachEventSource(DebugEventSource& other) override;

        /// <summary>
        /// Adds the incoming event.
        /// </summary>
        /// <param name="event">The event.</param>
        virtual void sendEvent(IncomingEventContextPtr const& event) override;

        void SetLevelFilter(uint8_t defaultLevel, uint8_t levelMin, uint8_t levelMax) override;

        void SetLevelFilter(uint8_t defaultLevel, const std::set<uint8_t>& allowedLevels) override;

        virtual IDataViewerCollection& GetDataViewerCollection() override;
        virtual const IDataViewerCollection& GetDataViewerCollection() const override;

        /// <summary>
        /// Get a reference to this log manager diagnostic level filter
        /// </summary>
        virtual const DiagLevelFilter& GetLevelFilter() override;

        /// <summary>
        /// Get a reference to this log manager instance ContextFieldsProvider
        /// </summary>
        /// <param name="level">Diagnostic level.</param>
        virtual const ContextFieldsProvider& GetContext() override
        {
            return m_context;
        }

        static size_t GetDeadLoggerCount();

       protected:
        std::unique_ptr<ITelemetrySystem>& GetSystem();
        void InitializeModules() noexcept;
        void TeardownModules() noexcept;

        MATSDK_LOG_DECL_COMPONENT_CLASS();

        static DeadLoggers s_deadLoggers;
        std::recursive_mutex m_lock;
        LoggerMap m_loggers;
        ContextFieldsProvider m_context;

        std::shared_ptr<IHttpClient> m_httpClient;
        std::shared_ptr<ITaskDispatcher> m_taskDispatcher;
        std::shared_ptr<IDataViewer> m_dataViewer;

        std::unique_ptr<IRuntimeConfig> m_config;
        ILogConfiguration& m_logConfiguration;

        IBandwidthController* m_bandwidthController;
        std::unique_ptr<IBandwidthController> m_ownBandwidthController;

        AuthTokensController m_authTokensController;

        std::unique_ptr<IOfflineStorage> m_offlineStorage;
        std::unique_ptr<LogSessionDataProvider> m_logSessionDataProvider;
        bool m_isSystemStarted{};
        std::unique_ptr<ITelemetrySystem> m_system;

        bool m_alive;

        DebugEventSource m_debugEventSource;
        DiagLevelFilter m_diagLevelFilter;

        EventFilterCollection m_filters;
        std::vector<std::unique_ptr<IModule>> m_modules;
        DataViewerCollection m_dataViewerCollection;
    };

}
MAT_NS_END

#endif
