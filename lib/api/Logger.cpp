// Copyright (c) Microsoft. All rights reserved.

#include "Logger.hpp"
#include "api/CommonLogManagerInternal.hpp"
#include "LogSessionData.hpp"
#include "LogManagerImpl.hpp"
#include "utils/Utils.hpp"
#include <algorithm>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  {
// *INDENT-ON*

//    extern void SendAsJSON(const EventProperties& properties, const std::string& token);

Logger::Logger(std::string const& tenantToken, std::string const& source, std::string const& experimentationProject,
    ILogManagerInternal* logManager, ContextFieldsProvider* parentContext, IRuntimeConfig* runtimeConfig)
  : m_lockP(new std::mutex()),
    m_tenantTokenP(new std::string(tenantToken)),
    m_sourceP(new std::string(source)),
    m_logManagerP(logManager),
    m_context(new ContextFieldsProvider(parentContext)),
    m_baseDecorator(new BaseDecorator(source)),
    m_runtimeConfigDecorator(new RuntimeConfigDecorator(runtimeConfig, tenantTokenToId(tenantToken), experimentationProject)),
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
        CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_REJECTED);
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

void Logger::SetContext(const std::string& k, const char       v[], PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, const std::string& v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, double             v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, int64_t            v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, time_ticks_t       v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, GUID_t             v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };
void Logger::SetContext(const std::string& k, bool               v, PiiKind pii) { SetContext(k, EventProperty(v, pii)); };

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

#if 1 /* Send to shadow */
 //   SendAsJSON(properties, *m_tenantTokenP);
#endif

    submit(record, latency, properties.GetPersistence(), properties.GetPolicyBitFlags());
    CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_LOG_EVENT);
}

bool Logger::applyCommonDecorators(::AriaProtocol::CsEvent& record, EventProperties const& properties, ::Microsoft::Applications::Events ::EventLatency& latency)
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
                    ::Microsoft::Applications::Events ::EventLatency latency, 
                    ::Microsoft::Applications::Events ::EventPersistence persistence, 
                    std::uint64_t  const& policyBitFlags)
{
    if (latency == EventLatency_Off)
    {
        CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_DROPPED);
        ARIASDK_LOG_INFO("Event %s/%s dropped because of calculated priority 0 (Off)",
            tenantTokenToId(*m_tenantTokenP).c_str(), record.baseType.c_str());
        return;
    }

    IncomingEventContextPtr event = IncomingEventContext::create(PAL::generateUuidString(), *m_tenantTokenP, latency, persistence, &record);
    event->policyBitFlags = policyBitFlags;
    if (m_logManagerP)
    {
        m_logManagerP->addIncomingEvent(event);
    }
    else
    {
        CommonLogManagerInternal::AddIncomingEvent(event);
    }
}

}}} // namespace Microsoft::Applications::Events 
