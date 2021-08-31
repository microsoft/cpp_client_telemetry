//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "pal/PAL.hpp"

#include "api/IRuntimeConfig.hpp"

#include "ILogManager.hpp"
#include "LogManagerImpl.hpp"

#include "ILogger.hpp"

#include "ContextFieldsProvider.hpp"

// Decorators
#include "decorators/BaseDecorator.hpp"
#include "decorators/EventPropertiesDecorator.hpp"
#include "decorators/SemanticApiDecorators.hpp"
#include "decorators/SemanticContextDecorator.hpp"

#include "filter/EventFilterCollection.hpp"

namespace MAT_NS_BEGIN
{
    class BaseDecorator;
    class ILogManagerInternal;

    class ActiveLoggerCall;

    class Logger : public ILogger,
                   public IContextProvider,
                   public DebugEventDispatcher
    {
       public:
        Logger(
            const std::string& tenantToken,
            const std::string& source,
            const std::string& scope,
            ILogManagerInternal& logManager,
            ContextFieldsProvider& parentContext,
            IRuntimeConfig& runtimeConfig);
        ~Logger() noexcept;

       public:
        virtual void SetContext(const std::string& name,
                                const char value[],
                                PiiKind piiKind = PiiKind_None) override;

        virtual void SetContext(const std::string& name,
                                const std::string& value,
                                PiiKind piiKind = PiiKind_None) override;

        virtual void SetContext(const std::string& name,
                                double value,
                                PiiKind piiKind = PiiKind_None) override;

        virtual void SetContext(const std::string& name,
                                int64_t value,
                                PiiKind piiKind = PiiKind_None) override;

        virtual void SetContext(const std::string& name,
                                bool value,
                                PiiKind piiKind = PiiKind_None) override;

        virtual void SetContext(const std::string& name,
                                time_ticks_t value,
                                PiiKind piiKind = PiiKind_None) override;

        virtual void SetContext(const std::string& name,
                                GUID_t value,
                                PiiKind piiKind = PiiKind_None) override;

        virtual void SetContext(const std::string& name,
                                int8_t value,
                                PiiKind piiKind = PiiKind_None) override
        {
            SetContext(name, (int64_t)value, piiKind);
        }

        virtual void SetContext(const std::string& name,
                                int16_t value,
                                PiiKind piiKind = PiiKind_None) override
        {
            SetContext(name, (int64_t)value, piiKind);
        }

        virtual void SetContext(const std::string& name,
                                int32_t value,
                                PiiKind piiKind = PiiKind_None) override
        {
            SetContext(name, (int64_t)value, piiKind);
        }

        virtual void SetContext(const std::string& name,
                                uint8_t value,
                                PiiKind piiKind = PiiKind_None) override
        {
            SetContext(name, (int64_t)value, piiKind);
        }

        virtual void SetContext(const std::string& name,
                                uint16_t value,
                                PiiKind piiKind = PiiKind_None) override
        {
            SetContext(name, (int64_t)value, piiKind);
        }

        virtual void SetContext(const std::string& name,
                                uint32_t value,
                                PiiKind piiKind = PiiKind_None) override
        {
            SetContext(name, (int64_t)value, piiKind);
        }

        virtual void SetContext(const std::string& name,
                                uint64_t value,
                                PiiKind piiKind = PiiKind_None) override
        {
            SetContext(name, (int64_t)value, piiKind);
        }

        virtual void
        SetContext(const std::string& name, const EventProperty& prop) override;

        virtual void SetLevel(uint8_t level) override;

        virtual ISemanticContext* GetSemanticContext() const override;

        virtual void SetParentContext(ISemanticContext* context) final;

        virtual void LogAppLifecycle(AppLifecycleState state,
                                     EventProperties const& properties) override;

        virtual void LogSession(SessionState state,
                                const EventProperties& properties) override;

        virtual void LogEvent(std::string const& name) override;

        virtual void LogEvent(EventProperties const& properties) override;

        virtual void LogFailure(std::string const& signature,
                                std::string const& detail,
                                std::string const& category,
                                std::string const& id,
                                EventProperties const& properties) override;

        virtual void LogFailure(std::string const& signature,
                                std::string const& detail,
                                EventProperties const& properties) override;

        virtual void LogPageView(std::string const& id,
                                 std::string const& pageName,
                                 std::string const& category,
                                 std::string const& uri,
                                 std::string const& referrerUri,
                                 EventProperties const& properties) override;

        virtual void LogPageView(std::string const& id,
                                 std::string const& pageName,
                                 EventProperties const& properties) override;

        virtual void LogPageAction(std::string const& pageViewId,
                                   ActionType actionType,
                                   EventProperties const& properties) override;

        virtual void LogPageAction(PageActionData const& pageActionData,
                                   EventProperties const& properties) override;

        virtual void LogSampledMetric(std::string const& name,
                                      double value,
                                      std::string const& units,
                                      std::string const& instanceName,
                                      std::string const& objectClass,
                                      std::string const& objectId,
                                      EventProperties const& properties) override;

        virtual void LogSampledMetric(std::string const& name,
                                      double value,
                                      std::string const& units,
                                      EventProperties const& properties) override;

        virtual void LogAggregatedMetric(std::string const& name,
                                         long duration,
                                         long count,
                                         EventProperties const& properties) override;

        virtual void LogAggregatedMetric(AggregatedMetricData const& metricData,
                                         EventProperties const& properties) override;

        virtual void LogTrace(TraceLevel level,
                              std::string const& message,
                              EventProperties const& properties) override;

        virtual void LogUserState(UserState state,
                                  long timeToLiveInMillis,
                                  EventProperties const& properties) override;

        virtual IEventFilterCollection& GetEventFilters() noexcept override;

        virtual IEventFilterCollection const&
        GetEventFilters() const noexcept override;

        virtual std::string GetSource();

        virtual ILogManager& GetParent();

        /// <summary>
        /// Gets the log session data.
        /// </summary>
        /// <returns>The log session data in a pointer to a LogSessionData object.</returns>
        virtual LogSessionData* GetLogSessionData() override;

        /// <summary>
        /// Get the Auth ticket controller
        /// </summary>
        virtual IAuthTokensController* GetAuthTokensController() override;

        virtual bool DispatchEvent(DebugEvent evt) override;

        virtual void onSubmitted();

        /// <summary>Switch from active to shut-down state</summary>
        virtual void RecordShutdown();

       protected:
        bool applyCommonDecorators(::CsProtocol::Record& record,
                                   EventProperties const& properties,
                                   MAT::EventLatency& latency);

        virtual void
        submit(::CsProtocol::Record& record, const EventProperties& props);

        bool
        CanEventPropertiesBeSent(EventProperties const& properties) const noexcept;

        std::mutex m_lock;

        std::string m_tenantToken;
        std::string m_iKey;
        std::string m_source;

        // Scope values:
        // "-"      - allows C API caller to detach their guest ILogger from parent's Host global context (default)
        // "*"      - allows C API caller to attach their guest ILogger to parent's Host global context
        // "<id>"   - allows to rewire this ILogger to alternate semantic context
        std::string m_scope;
        uint8_t m_level;

        ILogManagerInternal& m_logManager;
        ContextFieldsProvider m_context;
        IRuntimeConfig& m_config;

        BaseDecorator m_baseDecorator;
        EventPropertiesDecorator m_eventPropertiesDecorator;
        SemanticContextDecorator m_semanticContextDecorator;
        SemanticApiDecorators m_semanticApiDecorators;

        int64_t m_sessionStartTime;
        std::string m_sessionId;

        bool m_allowDotsInType;
        std::string m_customTypePrefix;

        bool m_resetSessionOnEnd;
        EventFilterCollection m_filters;

        /// m_shutdown_mutex protects shut-down state
        mutable std::mutex m_shutdown_mutex;

        /// RecordShutdown() uses m_shutdown_condition to wait until
        /// m_active goes to zero.
        mutable std::condition_variable m_shutdown_condition;
        /// m_active count records how many calls into this logger
        /// are in progress. We can only complete the shutdown state
        /// transition when active_count drains to zero.
        mutable size_t m_active_count = 0;
        /// m_active is set to false when we start the shut-down state
        /// transition. No new calls will start once this is false, so
        /// m_active_count should decrement to zero as calls complete.
        mutable bool m_active = true;

        /// ActiveLoggerCall is a stack-allocated class to handle
        /// shut-down state for individual Logger methods: increment
        /// and decrement m_active_count as needed, record whether
        /// this method call is in the active or shut-down state.
        friend class ActiveLoggerCall;
    };
}
MAT_NS_END

#endif

