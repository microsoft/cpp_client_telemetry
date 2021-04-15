//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "Logger.hpp"
#include "CommonFields.h"
#include "LogSessionData.hpp"
#include "NullObjects.hpp"
#include "utils/Utils.hpp"

#include <algorithm>
#include <array>

using namespace MAT;

namespace MAT_NS_BEGIN
{
    class ActiveLoggerCall
    {
       public:
        const Logger& m_logger;
        bool m_active;
        bool m_unpaused;

        ActiveLoggerCall(ActiveLoggerCall const& source) : m_logger(source.m_logger)
        {
            m_unpaused = m_logger.m_logManager.StartActivity();
            std::lock_guard<std::mutex> lock(m_logger.m_shutdown_mutex);
            m_active = m_logger.m_active;
            if (m_active)
            {
                m_logger.m_active_count += 1;
            }
        }

        /// Record current state on construction; increment active
        /// count if we are active.
        explicit ActiveLoggerCall(const Logger& parent) :
            m_logger(parent)
        {
            m_unpaused = m_logger.m_logManager.StartActivity();
            std::lock_guard<std::mutex> lock(m_logger.m_shutdown_mutex);
            m_active = m_logger.m_active;
            if (m_active)
            {
                m_logger.m_active_count += 1;
            }
        }

        /// If active, decrement active count, wake any listeners when
        /// active count reaches zero (usually there are no listeners).
        ~ActiveLoggerCall()
        {
            if (m_unpaused)
            {
                m_logger.m_logManager.EndActivity();
            }
            if (m_active)
            {
                std::lock_guard<std::mutex> lock(m_logger.m_shutdown_mutex);

                if (m_logger.m_active_count > 0)
                {
                    m_logger.m_active_count -= 1;
                    if (m_logger.m_active_count == 0)
                    {
                        const_cast<std::condition_variable*>(&m_logger.m_shutdown_condition)->notify_all();
                    }
                }
            }
        }

        bool LoggerIsDead() const noexcept
        {
            return !m_active || !m_unpaused;
        }
    };

    static NullLogManager nullManager;

    Logger::Logger(
        const std::string& tenantToken,
        const std::string& source,
        const std::string& scope,

        ILogManagerInternal& logManager,
        ContextFieldsProvider& parentContext,
        IRuntimeConfig& runtimeConfig) :
        m_tenantToken(tenantToken),
        m_source(source),
        // TODO: scope parameter can be used to rewire the logger to alternate context.
        // Scope must uniquely identify the "shared context" instance id.
        m_scope(scope),
        m_level(DIAG_LEVEL_DEFAULT),
        m_logManager(logManager),
        m_context(&parentContext),
        m_config(runtimeConfig),

        m_baseDecorator(logManager),
        m_eventPropertiesDecorator(logManager),
        m_semanticContextDecorator(logManager, m_context),
        m_semanticApiDecorators(logManager),
        m_sessionStartTime(0),
        m_allowDotsInType(false),
        m_resetSessionOnEnd(false)
    {
        std::string tenantId = tenantTokenToId(m_tenantToken);
        LOG_TRACE("%p: New instance (tenantId=%s)", this, tenantId.c_str());
        m_iKey = "o:" + tenantId;
        if (m_config.HasConfig(CFG_MAP_COMPAT))
        {
            MAT::VariantMap& cfg = m_config[CFG_MAP_COMPAT];
            m_allowDotsInType = cfg[CFG_BOOL_COMPAT_DOTS];
            m_customTypePrefix = static_cast<std::string&>(cfg[CFG_STR_COMPAT_PREFIX]);
        }
        m_resetSessionOnEnd = m_config[CFG_BOOL_SESSION_RESET_ENABLED];

        // Special scope "-" - means opt-out from parent context variables auto-capture.
        // It allows to detach the logger from its parent context.
        // This is the default mode for C API guests.
        if (m_scope == CONTEXT_SCOPE_NONE)
        {
            SetParentContext(nullptr);
        }
    }

    Logger::~Logger() noexcept
    {
        LOG_TRACE("%p: Destroyed", this);
    }

    ISemanticContext* Logger::GetSemanticContext() const
    {
        return (ISemanticContext*)(&m_context);
    }

    /******************************************************************************
    * Logger::SetContext
    *
    * Set app/session context fields.
    *
    * Could be used to overwrite the auto-populated(Part A) context fields
    * (ie. m_commonContextFields)
    *
    ******************************************************************************/
    void Logger::SetContext(const std::string& name, const EventProperty& prop)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: SetContext( properties.name=\"%s\", properties.value=\"%s\", PII=%u, ...)",
                  this, name.c_str(), prop.to_string().c_str(), prop.piiKind);

        const EventRejectedReason isValidPropertyName = validatePropertyName(name);
        if (isValidPropertyName != REJECTED_REASON_OK)
        {
            LOG_ERROR("Context name is invalid: %s", name.c_str());
            DebugEvent evt;
            evt.type = DebugEventType::EVT_REJECTED;
            evt.param1 = isValidPropertyName;
            DispatchEvent(evt);
            return;
        }

        // Always overwrite the stored value.
        // Empty string is allowed to remove the previously set value.
        // If the value is empty, the context will not be added to event.
        m_context.SetCustomField(name, prop);
    }

    void Logger::SetContext(const std::string& k, const char v[], PiiKind pii)
    {
        SetContext(k, EventProperty(v, pii));
    };

    void Logger::SetContext(const std::string& k, const std::string& v, PiiKind pii)
    {
        SetContext(k, EventProperty(v, pii));
    };

    void Logger::SetContext(const std::string& k, double v, PiiKind pii)
    {
        SetContext(k, EventProperty(v, pii));
    };

    void Logger::SetContext(const std::string& k, int64_t v, PiiKind pii)
    {
        SetContext(k, EventProperty(v, pii));
    };

    void Logger::SetContext(const std::string& k, time_ticks_t v, PiiKind pii)
    {
        SetContext(k, EventProperty(v, pii));
    };

    void Logger::SetContext(const std::string& k, GUID_t v, PiiKind pii)
    {
        SetContext(k, EventProperty(v, pii));
    };

    void Logger::SetContext(const std::string& k, bool v, PiiKind pii)
    {
        SetContext(k, EventProperty(v, pii));
    };

    // The goal of this method is to rewire the logger instance to any other ISemanticContext issued by SDK.
    // SDK may provide a future option for a guest logger to opt-in into its own semantic context. The method will then
    // rewire from the default parent (Host LogManager context) to guest's sandbox context, i.e. enabling scenario where
    // several guests are attached to one host, but each guest has their own 'local' LogManager semantic context sandbox.
    // ...
    // LogManager<T>::SetContext(...); // issued by guests would also allow to set context variable on guest's sandbox.
    //
    // C API does not expose shared context to the callers. Default option for C API 'guest' customers is to detach them
    // from the parent logger via ILogger::SetParentContext(nullptr)
    //
    void Logger::SetParentContext(ISemanticContext* context)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        if (context == nullptr)
        {
            // Since common props would typically be populated by the root-level
            // LogManager instance and we are detaching from that one, we need
            // to populate this context with common props directly.
            PAL::registerSemanticContext(&m_context);
        }
        m_context.SetParentContext(static_cast<ContextFieldsProvider*>(context));
    }

    /// <summary>
    /// Logs the application lifecycle.
    /// </summary>
    /// <param name="state">The state.</param>
    /// <param name="properties">The properties.</param>
    void Logger::LogAppLifecycle(AppLifecycleState state, EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: LogAppLifecycle(state=%u, properties.name=\"%s\", ...)",
                  this, state, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        ::CsProtocol::Record record;

        const bool decorated =
            applyCommonDecorators(record, properties, latency) &&
            m_semanticApiDecorators.decorateAppLifecycleMessage(record, state);
        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "AppLifecycle", tenantTokenToId(m_tenantToken).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_LIFECYCLE, size_t(latency), size_t(0), static_cast<void*>(&record), sizeof(record)));
    }

    /// <summary>
    /// Logs the custom event with the specified name.
    /// </summary>
    /// <param name="name">A string that contains the name of the custom event.</param>
    void Logger::LogEvent(std::string const& name)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        EventProperties event(name);
        LogEvent(event);
    }

    /// <summary>
    /// Logs the event.
    /// </summary>
    /// <param name="properties">The properties.</param>
    void Logger::LogEvent(EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        // SendAsJSON(properties, m_tenantToken);

        LOG_TRACE("%p: LogEvent(properties.name=\"%s\", ...)",
                  this, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        if (properties.GetLatency() > EventLatency_Unspecified)
        {
            latency = properties.GetLatency();
        }

        ::CsProtocol::Record record;

        if (!applyCommonDecorators(record, properties, latency))
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "custom",
                      tenantTokenToId(m_tenantToken).c_str(),
                      properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_EVENT, size_t(latency), size_t(0), static_cast<void*>(&record), sizeof(record)));
    }

    /// <summary>
    /// Logs a failure event - such as an application exception.
    /// </summary>
    /// <param name="signature">A string that contains the signature that identifies the bucket of the failure.</param>
    /// <param name="detail">A string that contains a description of the failure.</param>
    /// <param name="category">A string that contains the category of the failure - such as an application error,
    /// application not responding, or application crash</param>
    /// <param name="id">A string that contains the identifier that uniquely identifies this failure.</param>
    /// <param name="properties">Properties of this failure event, specified using an EventProperties object.</param>
    void Logger::LogFailure(
        std::string const& signature,
        std::string const& detail,
        std::string const& category,
        std::string const& id,
        EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: LogFailure(signature=\"%s\", properties.name=\"%s\", ...)",
                  this, signature.c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        ::CsProtocol::Record record;

        const bool decorated =
            applyCommonDecorators(record, properties, latency) &&
            m_semanticApiDecorators.decorateFailureMessage(record, signature, detail, category, id);

        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "Failure",
                      tenantTokenToId(m_tenantToken).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_FAILURE, size_t(latency), size_t(0), static_cast<void*>(&record), sizeof(record)));
    }

    void Logger::LogFailure(
        std::string const& signature,
        std::string const& detail,
        EventProperties const& properties)
    {
        // this inner call to LogFailure will instantiate ActiveCall
        LogFailure(signature, detail, "", "", properties);
    }

    void Logger::LogPageView(
        std::string const& id,
        std::string const& pageName,
        std::string const& category,
        std::string const& uri,
        std::string const& referrer,
        EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: LogPageView(id=\"%s\", properties.name=\"%s\", ...)",
                  this, id.c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        ::CsProtocol::Record record;

        const bool decorated =
            applyCommonDecorators(record, properties, latency) &&
            m_semanticApiDecorators.decoratePageViewMessage(record, id, pageName, category, uri, referrer);

        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "PageView", tenantTokenToId(m_tenantToken).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_PAGEVIEW, size_t(latency), size_t(0), (void*)(&record), sizeof(record)));
    }

    void Logger::LogPageView(
        std::string const& id,
        std::string const& pageName,
        EventProperties const& properties)
    {
        LogPageView(id, pageName, "", "", "", properties);
    }

    void Logger::LogPageAction(
        std::string const& pageViewId,
        ActionType actionType,
        EventProperties const& properties)
    {
        PageActionData pageActionData(pageViewId, actionType);
        LogPageAction(pageActionData, properties);
    }

    void Logger::LogPageAction(
        PageActionData const& pageActionData,
        EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: LogPageAction(pageActionData.actionType=%u, properties.name=\"%s\", ...)",
                  this, pageActionData.actionType, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        ::CsProtocol::Record record;

        const bool decorated =
            applyCommonDecorators(record, properties, latency) &&
            m_semanticApiDecorators.decoratePageActionMessage(record, pageActionData);
        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "PageAction", tenantTokenToId(m_tenantToken).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_PAGEACTION, size_t(latency), size_t(0), (void*)(&record), sizeof(record)));
    }

    /// <summary>
    /// Applies the common decorators.
    /// </summary>
    /// <param name="record">The record.</param>
    /// <param name="properties">The properties.</param>
    /// <param name="latency">The latency.</param>
    /// <returns></returns>
    bool Logger::applyCommonDecorators(::CsProtocol::Record& record, EventProperties const& properties, EventLatency& latency)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return false;
        }

        record.name = properties.GetName();
        record.baseType = m_customTypePrefix;

        std::string evtType = properties.GetType();
        if (!evtType.empty())
        {
            if (!record.baseType.empty())
            {
                record.baseType.append(".");
            }
            if (!m_allowDotsInType)
            {
                std::replace(evtType.begin(), evtType.end(), '.', '_');
            }
            record.baseType.append(evtType);
        }

        if (record.name.empty())
        {
            record.name = "NotSpecified";
        }
        record.iKey = m_iKey;

        return m_baseDecorator.decorate(record) && m_semanticContextDecorator.decorate(record) && m_eventPropertiesDecorator.decorate(record, latency, properties);
    }

    void Logger::submit(::CsProtocol::Record& record, const EventProperties& props)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        const auto policyBitFlags = props.GetPolicyBitFlags();
        const auto persistence = props.GetPersistence();
        const auto latency = props.GetLatency();
        auto levelFilter = m_logManager.GetLevelFilter();
        if (levelFilter.IsLevelFilterEnabled())
        {
            const auto& m_props = props.GetProperties();
            const auto it = m_props.find(COMMONFIELDS_EVENT_LEVEL);
            //
            // Level policy:
            // * get level from the COMMONFIELDS_EVENT_LEVEL property if set
            // * if not set, then get level from the ILogger instance
            // * if not set, then get level from the LogManager instance
            // * if still not set (no default assigned at LogManager scope),
            // then prefer to drop. This is user error: user set the range
            // restrition, but didn't specify the defaults.
            //
            uint8_t level = (it != m_props.cend()) ? static_cast<uint8_t>(it->second.as_int64) : m_level;
            if (level == DIAG_LEVEL_DEFAULT)
            {
                level = levelFilter.GetDefaultLevel();
                if (level == DIAG_LEVEL_DEFAULT)
                {
                    // If no default level, but restrictions are in effect, then prefer to drop event
                    LOG_INFO("Event %s/%s dropped: no diagnostic level assigned!",
                             tenantTokenToId(m_tenantToken).c_str(), record.baseType.c_str());
                    DispatchEvent(DebugEventType::EVT_FILTERED);
                    return;
                }
            }
            if (!levelFilter.IsLevelEnabled(level))
            {
                DispatchEvent(DebugEventType::EVT_FILTERED);
                return;
            }
        }

        if (latency == EventLatency_Off)
        {
            DispatchEvent(DebugEventType::EVT_DROPPED);
            LOG_INFO("Event %s/%s dropped: calculated latency 0 (Off)",
                     tenantTokenToId(m_tenantToken).c_str(), record.baseType.c_str());
            return;
        }

        // TODO: [MG] - check if optimization is possible in generateUuidString
        IncomingEventContext event(PAL::generateUuidString(), m_tenantToken, latency, persistence, &record);
        event.policyBitFlags = policyBitFlags;

        m_logManager.sendEvent(&event);
    }

    void Logger::onSubmitted()
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_INFO("This method is executed from worker thread");
    }

    void Logger::LogSampledMetric(
        std::string const& name,
        double value,
        std::string const& units,
        std::string const& instanceName,
        std::string const& objectClass,
        std::string const& objectId,
        EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: LogSampledMetric(name=\"%s\", properties.name=\"%s\", ...)",
                  this, name.c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        ::CsProtocol::Record record;

        const bool decorated =
            applyCommonDecorators(record, properties, latency) &&
            m_semanticApiDecorators.decorateSampledMetricMessage(record, name, value, units, instanceName, objectClass, objectId);

        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "SampledMetric", tenantTokenToId(m_tenantToken).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_SAMPLEMETR, size_t(latency), size_t(0), (void*)(&record), sizeof(record)));
    }

    void Logger::LogSampledMetric(
        std::string const& name,
        double value,
        std::string const& units,
        EventProperties const& properties)
    {
        LogSampledMetric(name, value, units, "", "", "", properties);
    }

    void Logger::LogAggregatedMetric(
        std::string const& name,
        long duration,
        long count,
        EventProperties const& properties)
    {
        AggregatedMetricData metricData(name, duration, count);
        LogAggregatedMetric(metricData, properties);
    }

    void Logger::LogAggregatedMetric(
        AggregatedMetricData const& metricData,
        EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: LogAggregatedMetric(name=\"%s\", properties.name=\"%s\", ...)",
                  this, metricData.name.c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        ::CsProtocol::Record record;

        const bool decorated =
            applyCommonDecorators(record, properties, latency) &&
            m_semanticApiDecorators.decorateAggregatedMetricMessage(record, metricData);

        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "AggregatedMetric", tenantTokenToId(m_tenantToken).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_AGGRMETR, size_t(latency), size_t(0), (void*)(&record), sizeof(record)));
    }

    void Logger::LogTrace(
        TraceLevel level,
        std::string const& message,
        EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: LogTrace(level=%u, properties.name=\"%s\", ...)",
                  this, level, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        ::CsProtocol::Record record;

        bool decorated =
            applyCommonDecorators(record, properties, latency) &&
            m_semanticApiDecorators.decorateTraceMessage(record, level, message);

        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "Trace", tenantTokenToId(m_tenantToken).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_TRACE, size_t(latency), size_t(0), (void*)(&record), sizeof(record)));
    }

    void Logger::LogUserState(
        UserState state,
        long timeToLiveInMillis,
        EventProperties const& properties)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        LOG_TRACE("%p: LogUserState(state=%u, properties.name=\"%s\", ...)",
                  this, state, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

        if (!CanEventPropertiesBeSent(properties))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        EventLatency latency = EventLatency_Normal;
        ::CsProtocol::Record record;

        bool decorated =
            applyCommonDecorators(record, properties, latency) &&
            m_semanticApiDecorators.decorateUserStateMessage(record, state, timeToLiveInMillis);

        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "UserState", tenantTokenToId(m_tenantToken).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
            return;
        }

        submit(record, properties);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_USERSTATE, size_t(latency), size_t(0), (void*)(&record), sizeof(record)));
    }

    /******************************************************************************
    * Logger::LogSession
    *
    * Log a user's Session.
    *
    ******************************************************************************/
    void Logger::LogSession(SessionState state, const EventProperties& props)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return;
        }

        if (!CanEventPropertiesBeSent(props))
        {
            DispatchEvent(DebugEventType::EVT_FILTERED);
            return;
        }

        auto logSessionData = m_logManager.GetLogSessionData();
        std::string sessionSDKUid;
        unsigned long long sessionFirstTime = 0;
        if (logSessionData!=nullptr)
        {
            sessionSDKUid = logSessionData->getSessionSDKUid();
            sessionFirstTime = logSessionData->getSessionFirstTime();
        }

        if (sessionSDKUid == "" || sessionFirstTime == 0)
        {
            LOG_WARN("We don't have a first time so no session logged");
            return;
        }

        EventRejectedReason isValidEventName = validateEventName(props.GetName());
        if (isValidEventName != REJECTED_REASON_OK)
        {
            LOG_ERROR("Invalid event properties!");
            DebugEvent evt;
            evt.type = DebugEventType::EVT_REJECTED;
            evt.param1 = isValidEventName;
            DispatchEvent(evt);
            return;
        }

        int64_t sessionDuration = 0;
        switch (state)
        {
        case SessionState::Session_Started:
        {
            if (m_sessionStartTime > 0)
            {
                LOG_ERROR("LogSession The order is not the correct one in calling LogSession");
                return;
            }
            m_sessionStartTime = PAL::getUtcSystemTime();

            m_sessionId = PAL::generateUuidString();
            break;
        }
        case SessionState::Session_Ended:
        {
            if (m_sessionStartTime == 0)
            {
                LOG_WARN("LogSession We don't have session start time");
                return;
            }
            sessionDuration = PAL::getUtcSystemTime() - m_sessionStartTime;

            if (m_resetSessionOnEnd)
            {
                // reset the time of the session to 0 and get a new sessionId
                m_sessionStartTime = 0;
                if (logSessionData!=nullptr)
                {
                    m_logManager.ResetLogSessionData();
                    LOG_TRACE("Resetting session data on session end");
                }
            }

            break;
        }
        }

        EventLatency latency = EventLatency_RealTime;
        ::CsProtocol::Record record;

        bool decorated = applyCommonDecorators(record, props, latency) &&
                         m_semanticApiDecorators.decorateSessionMessage(record, state, m_sessionId, PAL::formatUtcTimestampMsAsISO8601(sessionFirstTime), sessionSDKUid, sessionDuration);

        if (!decorated)
        {
            LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
                      "Trace", tenantTokenToId(m_tenantToken).c_str(), props.GetName().empty() ? "<unnamed>" : props.GetName().c_str());
            return;
        }

        submit(record, props);
        DispatchEvent(DebugEvent(DebugEventType::EVT_LOG_SESSION, size_t(latency), size_t(0), (void*)(&record), sizeof(record)));
    }

    IEventFilterCollection& Logger::GetEventFilters() noexcept
    {
        return m_filters;
    }

    const IEventFilterCollection& Logger::GetEventFilters() const noexcept
    {
        return m_filters;
    }

    ILogManager& Logger::GetParent()
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return nullManager;
        }

        return m_logManager;
    }

    LogSessionData* Logger::GetLogSessionData()
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return nullManager.GetLogSessionData();
        }

        return m_logManager.GetLogSessionData();
    }

    IAuthTokensController* Logger::GetAuthTokensController()
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return nullManager.GetAuthTokensController();
        }

        return m_logManager.GetAuthTokensController();
    }

    bool Logger::DispatchEvent(DebugEvent evt)
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return nullManager.DispatchEvent(std::move(evt));
        }

        return m_logManager.DispatchEvent(std::move(evt));
    }

    std::string Logger::GetSource()
    {
        return m_source;
    }

    void Logger::SetLevel(uint8_t level)
    {
        m_level = level;
    }

    bool Logger::CanEventPropertiesBeSent(EventProperties const& properties) const noexcept
    {
        ActiveLoggerCall active(*this);
        if (active.LoggerIsDead())
        {
            return false;
        }

        return m_filters.CanEventPropertiesBeSent(properties) && m_logManager.GetEventFilters().CanEventPropertiesBeSent(properties);
    }

    void Logger::RecordShutdown()
    {
        std::unique_lock<std::mutex> shutdownLock(m_shutdown_mutex);
        m_active = false;
        if (m_active_count > 0)
        {
            // wait for idle before continuing
            // as with all condition variables, we hold the lock,
            // so no thread can decrement m_active_count until
            // wait() releases (and later reacquires) the lock.
            m_shutdown_condition.wait(shutdownLock, [this]() {
                return m_active_count == 0; // we hold the lock for this call
            });
        }
    }
}
MAT_NS_END
