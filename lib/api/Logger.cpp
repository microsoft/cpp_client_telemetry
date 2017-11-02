// Copyright (c) Microsoft. All rights reserved.

#include "Logger.hpp"
#include "LogManager.hpp"
#include "LogSessionData.hpp"
#include "LogManagerImpl.hpp"
#include "utils/Utils.hpp"
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
    m_context(new ContextFieldsProvider(parentContext)),
    m_runtimeConfig(runtimeConfig),
    m_baseDecorator(new BaseDecorator(source)),
    m_runtimeConfigDecorator(new RuntimeConfigDecorator(m_runtimeConfig, tenantTokenToId(tenantToken), experimentationProject)),
    m_semanticContextDecorator(new SemanticContextDecorator(*m_context)),
    m_eventPropertiesDecorator( new EventPropertiesDecorator()),
    m_semanticApiDecorators( new SemanticApiDecorators()),
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
    if (m_baseDecorator) delete m_baseDecorator;
    if (m_runtimeConfigDecorator) delete m_runtimeConfigDecorator;
    if (m_semanticContextDecorator) delete m_semanticContextDecorator;
    if (m_eventPropertiesDecorator) delete m_eventPropertiesDecorator;
    if (m_semanticApiDecorators) delete m_semanticApiDecorators;
    if (m_context )delete m_context;
}

ISemanticContext* Logger::GetSemanticContext() const
{
    return (ISemanticContext*)m_context;
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
    if (m_context)
    {
        m_context->setCustomField(name, prop);
    }
}

void Logger::SetContext(const std::string& k, const char       v[], CustomerContentKind ccKind) { SetContext(k, EventProperty(v, ccKind)); }
void Logger::SetContext(const std::string& k, const std::string &v, CustomerContentKind ccKind) { SetContext(k, EventProperty(v, ccKind)); }
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

    EventLatency latency = EventLatency_Normal;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decorateAppLifecycleMessage(record, state)) ||
        !applyCommonDecorators(record, properties, latency))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "AppLifecycle", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
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

    EventLatency latency = EventLatency_Normal;
    if (properties.GetLatency() > EventLatency_Unspecified)
    {
        latency = properties.GetLatency();
    }
    
    ::AriaProtocol::CsEvent record;

    if (!applyCommonDecorators(record, properties, latency)) {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "custom", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
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

    EventLatency latency = EventLatency_Normal;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decorateFailureMessage(record, signature, detail, category, id)) ||
        !applyCommonDecorators(record, properties, latency))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "Failure", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
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

    EventLatency latency = EventLatency_Normal;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decoratePageViewMessage(record, id, pageName, category, uri, referrer)) ||
        !applyCommonDecorators(record, properties, latency))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "PageView", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
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

    EventLatency latency = EventLatency_Normal;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decoratePageActionMessage(record, pageActionData)) ||
        !applyCommonDecorators(record, properties, latency))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "PageAction", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
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

    EventLatency latency = EventLatency_Normal;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decorateSampledMetricMessage(record, name, value, units, instanceName, objectClass, objectId)) ||
        !applyCommonDecorators(record, properties, latency))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "SampledMetric", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
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

    EventLatency latency = EventLatency_Normal;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decorateAggregatedMetricMessage(record, metricData)) ||
        !applyCommonDecorators(record, properties, latency))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "AggregatedMetric", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_AGGRMETR);
}

void Logger::LogTrace(
    TraceLevel level,
    std::string const& message,
    EventProperties const& properties)
{
    ARIASDK_LOG_DETAIL("%p: LogTrace(level=%u, properties.name=\"%s\", ...)",
        this, level, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventLatency latency = EventLatency_Normal;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decorateTraceMessage(record, level, message)) ||
        !applyCommonDecorators(record, properties, latency))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "Trace", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_TRACE);
}

void Logger::LogUserState(
    UserState state,
    long timeToLiveInMillis,
    EventProperties const& properties)
{
    ARIASDK_LOG_DETAIL("%p: LogUserState(state=%u, properties.name=\"%s\", ...)",
        this, state, properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());

    EventLatency latency = EventLatency_Normal;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decorateUserStateMessage(record, state, timeToLiveInMillis)) ||
        !applyCommonDecorators(record, properties, latency))
    {
        ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
            "UserState", tenantTokenToId(*m_tenantTokenP).c_str(), properties.GetName().empty() ? "<unnamed>" : properties.GetName().c_str());
        return;
    }

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_USERSTATE);
}

bool Logger::applyCommonDecorators(::AriaProtocol::CsEvent& record, EventProperties const& properties, ::Microsoft::Applications::Telemetry::EventLatency& latency)
{
    record.name = properties.GetName();
    if (record.name.empty())
    {
        record.name = "NotSpecified";
    }
    record.iKey = "O:" + (*m_tenantTokenP).substr(0, (*m_tenantTokenP).find('-'));
    return  m_baseDecorator &&
            m_baseDecorator->decorate(record) &&
            m_semanticApiDecorators &&
            m_semanticContextDecorator->decorate(record) &&
            m_eventPropertiesDecorator &&
            m_eventPropertiesDecorator->decorate(record, latency, properties) &&
            m_runtimeConfigDecorator &&
            m_runtimeConfigDecorator->decorate(record);
           
}

void Logger::submit(::AriaProtocol::CsEvent& record, 
                    ::Microsoft::Applications::Telemetry::EventLatency latency, 
                    ::Microsoft::Applications::Telemetry::EventPersistence persistence, 
                    std::uint64_t  const& policyBitFlags)
{
    if (latency == EventLatency_Off)
    {
        LogManager::DispatchEvent(DebugEventType::EVT_DROPPED);
        ARIASDK_LOG_INFO("Event %s/%s dropped because of calculated priority 0 (Off)",
            tenantTokenToId(*m_tenantTokenP).c_str(), record.baseType.c_str());
        return;
    }

    IncomingEventContextPtr event = IncomingEventContext::create(PAL::generateUuidString(), *m_tenantTokenP, latency, persistence, &record);
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
            m_sessionStartTime = PAL::getUtcSystemTime();
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
            sessionDuration = PAL::getUtcSystemTime() - m_sessionStartTime;
            break;
        }
    }
    EventLatency latency = EventLatency_RealTime;
    ::AriaProtocol::CsEvent record;

    if ((m_semanticApiDecorators && !m_semanticApiDecorators->decorateSessionMessage(record, state, *m_sessionIdP, PAL::formatUtcTimestampMsAsISO8601(sessionFirstTime), sessionSDKUid, sessionDuration)) ||
        !applyCommonDecorators(record, prop, latency))
    {
       ARIASDK_LOG_ERROR("Failed to log %s event %s/%s: invalid arguments provided",
           "Trace", tenantTokenToId(*m_tenantTokenP).c_str(), prop.GetName().empty() ? "<unnamed>" : prop.GetName().c_str());
       return;
    }
        
    submit(record, latency, prop.GetPersistence(), prop.GetPolicyBitFlags());
    LogManager::DispatchEvent(DebugEventType::EVT_LOG_SESSION);
}

}}} // namespace Microsoft::Applications::Telemetry
