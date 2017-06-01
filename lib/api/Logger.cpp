// Copyright (c) Microsoft. All rights reserved.

#include "Logger.hpp"
#include "LogManager.hpp"
#include "LogManagerImpl.hpp"
#include "utils/Common.hpp"
#include <algorithm>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*


Logger::Logger(std::string const& tenantToken, std::string const& source, std::string const& experimentationProject,
    ILogManagerInternal& logManager, ContextFieldsProvider* parentContext, IRuntimeConfig& runtimeConfig)
  : m_lockP(new std::mutex()),
    m_tenantTokenP(new std::string(tenantToken)),
    m_sourceP(new std::string(source)),
    m_logManager(logManager),
    m_context(parentContext),
    m_runtimeConfig(runtimeConfig),
    m_baseDecorator(source),
    m_runtimeConfigDecorator(m_runtimeConfig, tenantTokenToId(tenantToken), experimentationProject),
    m_semanticContextDecorator(m_context),
    m_sessionStartTime(0),
    m_sessionIdP( new std::string(""))
{
    ARIASDK_LOG_DETAIL("%p: New instance (tenantId=%s)", this, tenantTokenToId(*m_tenantTokenP).c_str());
}

Logger::~Logger()
{
    ARIASDK_LOG_DETAIL("%p: Destructed", this);
    if (m_tenantTokenP) delete m_tenantTokenP;
    if (m_sourceP) delete m_sourceP;
    if (m_lockP) delete m_lockP;
    if (m_sessionIdP) delete m_sessionIdP;
}

ISemanticContext* Logger::GetSemanticContext() const
{
    return (ISemanticContext*)&m_context;
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
void Logger::SetContext(const std::string& name, EventProperty prop)
{
    ARIASDK_LOG_DETAIL("%p: SetContext( properties.name=\"%s\", properties.value=\"%s\", PII=%u, ...)",
        this, name.c_str(), prop.to_string().c_str(), prop.piiKind);

    if (!validatePropertyName(name)) 
    {
        LogManager::DispatchEvent(DebugEventType::EVT_REJECTED);
        ARIASDK_LOG_ERROR("Context name is invalid: %s", name.c_str());
        return;
    }
    // Always overwrite the stored value. 
    // Empty string is alowed to remove the previously set value.
    // If the value is empty, the context will not be added to event.
    m_context.setCustomField(name, prop);
}

void Logger::SetContext(const std::string& k, const char       v[], PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, const std::string& v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, double             v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, int64_t            v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, time_ticks_t       v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, GUID_t             v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, bool               v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };


void Logger::LogAppLifecycle(AppLifecycleState state, EventProperties const& properties)
{
    ARIASDK_LOG_DETAIL("%p: LogAppLifecycle(state=%u, properties.name=\"%s\", ...)",
        this, state, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decorateAppLifecycleMessage(record, state) ||
        !applyCommonDecorators(record, properties, priority))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "AppLifecycle", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_LIFECYCLE);
}

void Logger::LogEvent(std::string const& name)
{
    EventProperties event(name);
    LogEvent(event);
}

void Logger::LogEvent(EventProperties const& properties)
{
    ARIASDK_LOG_DETAIL("%p: LogEvent(properties.name=\"%s\", ...)",
        this, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    if (properties.GetPriority() > EventPriority_Unspecified)
    {
        priority = properties.GetPriority();
    }
    
    ::AriaProtocol::Record record;

    if (!applyCommonDecorators(record, properties, priority)) {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "custom", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_EVENT);
}

void Logger::LogFailure(
    std::string const& signature,
    std::string const& detail,
    std::string const& category,
    std::string const& id,
    EventProperties const& properties)
{
    ARIASDK_LOG_DETAIL("%p: LogFailure(signature=\"%s\", properties.name=\"%s\", ...)",
        this, signature.c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decorateFailureMessage(record, signature, detail, category, id) ||
        !applyCommonDecorators(record, properties, priority))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "Failure", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_FAILURE);
}

void Logger::LogFailure(
    std::string const& signature,
    std::string const& detail,
    EventProperties const& properties)
{
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
    ARIASDK_LOG_DETAIL("%p: LogPageView(id=\"%s\", properties.name=\"%s\", ...)",
        this, id.c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decoratePageViewMessage(record, id, pageName, category, uri, referrer) ||
        !applyCommonDecorators(record, properties, priority))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "PageView", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_PAGEVIEW);
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
    ARIASDK_LOG_DETAIL("%p: LogPageAction(pageActionData.actionType=%u, properties.name=\"%s\", ...)",
        this, pageActionData.actionType, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decoratePageActionMessage(record, pageActionData) ||
        !applyCommonDecorators(record, properties, priority))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "PageAction", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_PAGEACTION);
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
    ARIASDK_LOG_DETAIL("%p: LogSampledMetric(name=\"%s\", properties.name=\"%s\", ...)",
        this, name.c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decorateSampledMetricMessage(record, name, value, units, instanceName, objectClass, objectId) ||
        !applyCommonDecorators(record, properties, priority))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "SampledMetric", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_SAMPLEMETR);
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
    ARIASDK_LOG_DETAIL("%p: LogAggregatedMetric(name=\"%s\", properties.name=\"%s\", ...)",
        this, metricData.name.c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decorateAggregatedMetricMessage(record, metricData) ||
        !applyCommonDecorators(record, properties, priority))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "AggregatedMetric", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_AGGRMETR);
}

void Logger::LogTrace(
    TraceLevel level,
    std::string const& message,
    EventProperties const& properties)
{
    ARIASDK_LOG_DETAIL("%p: LogTrace(level=%u, properties.name=\"%s\", ...)",
        this, level, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decorateTraceMessage(record, level, message) ||
        !applyCommonDecorators(record, properties, priority))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "Trace", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_TRACE);
}

void Logger::LogUserState(
    UserState state,
    long timeToLiveInMillis,
    EventProperties const& properties)
{
    ARIASDK_LOG_DETAIL("%p: LogUserState(state=%u, properties.name=\"%s\", ...)",
        this, state, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventPriority priority = EventPriority_Normal;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decorateUserStateMessage(record, state, timeToLiveInMillis) ||
        !applyCommonDecorators(record, properties, priority))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "UserState", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, priority, properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_USERSTATE);
}

bool Logger::applyCommonDecorators(::AriaProtocol::Record& record, EventProperties const& properties, ::Microsoft::Applications::Telemetry::EventPriority& priority)
{
    return m_semanticContextDecorator.decorate(record) &&
           m_eventPropertiesDecorator.decorate(record, priority, properties) &&
           m_runtimeConfigDecorator.decorate(record, priority) &&
           m_baseDecorator.decorate(record, priority);
}

void Logger::submit(::AriaProtocol::Record& record, ::Microsoft::Applications::Telemetry::EventPriority priority, std::uint64_t  const& policyBitFlags)
{
    if (priority == EventPriority_Off)
    {
        LogManager::DispatchEvent(DebugEventType::EVT_DROPPED);
        ARIASDK_LOG_INFO("Event %s/%s dropped because of calculated priority 0 (Off)",
            tenantTokenToId(*m_tenantTokenP).c_str(), record.EventType.c_str());
        return;
    }

    IncomingEventContextPtr event = IncomingEventContext::create(record.Id, *m_tenantTokenP, priority, &record);
    event->policyBitFlags = policyBitFlags;
    m_logManager.addIncomingEvent(event);
}

/******************************************************************************
* Logger::LogSession
*
* Log a user's Session.
*
******************************************************************************/
void Logger::LogSession(SessionState state, const EventProperties& prop)
{
    LogSessionData* logSessionData = LogManager::GetLogSessionData();
    std::string sessionSDKUid = logSessionData->getSessionSDKUid();
    unsigned long long sessionFirstTime = logSessionData->getSesionFirstTime();

    if (sessionSDKUid == "" || sessionFirstTime == 0)
    {
        ARIASDK_LOG_WARNING("We don't have a first time so no session logged");
        return;
    }
    
    if (!validateEventName(prop.GetName()))
    {
        ARIASDK_LOG_ERROR("Invalid event properties!");
        LogManager::DispatchEvent(DebugEventType::EVT_REJECTED);
        return;
    }

    int64_t sessionDuration = 0;
    switch (state)
    {
        case SessionState::Session_Started:
        {
            if (m_sessionStartTime > 0)
            {
                ARIASDK_LOG_ERROR("LogSession The order is not the correct one in calling LogSession");
                return;
            }
            m_sessionStartTime = PAL::getUtcSystemTimeMs();
            if (m_sessionIdP) delete m_sessionIdP;
            m_sessionIdP = new std::string(PAL::generateUuidString());
            break;
        }
        case SessionState::Session_Ended:
        {
            if (m_sessionStartTime == 0)
            {
                ARIASDK_LOG_WARNING("LogSession We don't have session start time");
                return;
            }
            sessionDuration = (PAL::getUtcSystemTimeMs() - m_sessionStartTime) / 1000;
            break;
        }
    }
    EventPriority priority = EventPriority_High;
    ::AriaProtocol::Record record;

    if (!m_semanticApiDecorators.decorateSessionMessage(record, state, *m_sessionIdP, PAL::formatUtcTimestampMsAsISO8601(sessionFirstTime), sessionSDKUid, sessionDuration) ||
        !applyCommonDecorators(record, prop, priority))
    {
       ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
           "Trace", tenantTokenToId(*m_tenantTokenP).c_str(), prop.GetName().empty() ? "<unnamed>" : prop.GetName().c_str());
       return;
    }
        
    submit(record, priority, prop.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_SESSION);
}

}}} // namespace Microsoft::Applications::Telemetry
