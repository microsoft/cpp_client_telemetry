// Copyright (c) Microsoft. All rights reserved.
#ifndef LOGMANAGERIMPL_HPP
#define LOGMANAGERIMPL_HPP

#include <LogConfiguration.hpp>

#include "config/RuntimeConfig_Default.hpp"

#include "system/Contexts.hpp"

#include <IHttpClient.hpp>
#include <ILogManager.hpp>

#include "api/Logger.hpp"

#include <filter/EventFilterRegulator.hpp>

#include <DebugEvents.hpp>
#include <memory>

#include <IBandwidthController.hpp>
#include "api/AuthTokensController.hpp"

#include "LogSessionData.hpp"

#include <mutex>
#include <set>

namespace ARIASDK_NS_BEGIN {

    class ITelemetrySystem;

    class ILogManagerInternal : public ILogManager {

    public:

        static std::recursive_mutex     managers_lock;
        static std::set<ILogManager*>   managers;

        virtual void sendEvent(IncomingEventContextPtr const& event) = 0;

    };

    class Logger;

    class LogManagerImpl : public ILogManagerInternal {

    public:

        LogManagerImpl(ILogConfiguration& configuration);

        virtual ~LogManagerImpl() override;

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
        virtual status_t ResetTransmitProfiles();
        virtual const std::string& GetTransmitProfileName() override;

        /**
         * Semantic Context methods
         */
        virtual ISemanticContext& GetSemanticContext() override;

        virtual status_t SetContext(std::string const& name, std::string const& value, PiiKind piiKind = PiiKind_None) override;

        virtual status_t SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None) override;

        virtual status_t SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None) override;

        virtual inline status_t SetContext(const std::string& name, const char *value, PiiKind piiKind = PiiKind_None) override { const std::string val(value); return SetContext(name, val, piiKind); };

        virtual inline status_t SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None) override { return SetContext(name, (int64_t)value, piiKind); }

        virtual inline status_t SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) override { return SetContext(name, (int64_t)value, piiKind); }

        virtual inline status_t SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) override { return SetContext(name, (int64_t)value, piiKind); }

        virtual inline status_t SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None) override { return SetContext(name, (int64_t)value, piiKind); }

        virtual inline status_t SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) override { return SetContext(name, (int64_t)value, piiKind); }

        virtual inline status_t SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) override { return SetContext(name, (int64_t)value, piiKind); }

        virtual inline status_t SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) override { return SetContext(name, (int64_t)value, piiKind); }

        virtual status_t SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None) override;

        virtual status_t SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None) override;

        virtual status_t SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None) override;


        /**
         * GetLogger method
         */
        virtual ILogger* GetLogger(std::string const& tenantToken, std::string const& source = std::string(), std::string const& experimentationProject = std::string()) override;

        LogSessionData* GetLogSessionData() override;

        ILogController *GetLogController(void);

        IAuthTokensController* GetAuthTokensController() override;

        /// <summary>
        /// Adds the event listener.
        /// </summary>
        /// <param name="type">The type.</param>
        /// <param name="listener">The listener.</param>
        virtual void AddEventListener(DebugEventType type, DebugEventListener &listener) override;

        /// <summary>
        /// Removes the event listener.
        /// </summary>
        /// <param name="type">The type.</param>
        /// <param name="listener">The listener.</param>
        virtual void RemoveEventListener(DebugEventType type, DebugEventListener &listener) override;

        /// <summary>
        /// Dispatches the event.
        /// </summary>
        /// <param name="evt">The evt.</param>
        /// <returns></returns>
        virtual bool DispatchEvent(DebugEvent evt) override;

        ///
        virtual bool AttachEventSource(DebugEventSource & other) override;

        ///
        virtual bool DetachEventSource(DebugEventSource & other) override;
        
        /// <summary>
        /// Sets the exclusion filter.
        /// </summary>
        /// <param name="tenantToken">The tenant token.</param>
        /// <param name="filterStrings">The filter strings.</param>
        /// <param name="filterCount">The filter count.</param>
        /// <returns></returns>
        status_t SetExclusionFilter(const char* tenantToken, const char** filterStrings, uint32_t filterCount);
        

        /// <summary>
        /// Sets the exclusion filter.
        /// </summary>
        /// <param name="tenantToken">The tenant token.</param>
        /// <param name="filterStrings">The filter strings.</param>
        /// <param name="filterRates">The filter rates.</param>
        /// <param name="filterCount">The filter count.</param>
        /// <returns></returns>
        status_t SetExclusionFilter(const char* tenantToken, const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount);

        /// <summary>
        /// Adds the incoming event.
        /// </summary>
        /// <param name="event">The event.</param>
        virtual void sendEvent(IncomingEventContextPtr const& event) override;

    protected:

        ARIASDK_LOG_DECL_COMPONENT_CLASS();

        std::mutex                             m_lock;
        std::map<std::string, Logger*>         m_loggers;
        ContextFieldsProvider                  m_context;

        IHttpClient*                           m_httpClient;
        std::unique_ptr<IHttpClient>           m_ownHttpClient;

        IRuntimeConfig*                        m_config;
        ILogConfiguration&                     m_logConfiguration;

        IBandwidthController*                  m_bandwidthController;
        std::unique_ptr<IBandwidthController>  m_ownBandwidthController;

        AuthTokensController                   m_authTokensController;

        std::unique_ptr<IOfflineStorage>       m_offlineStorage;
        std::unique_ptr<LogSessionData>        m_logSessionData;
        std::unique_ptr<ITelemetrySystem>      m_system;

        EventFilterRegulator                   m_eventFilterRegulator;

        bool                                   m_alive;

        DebugEventSource                       m_debugEventSource;
    };


} ARIASDK_NS_END

#endif
