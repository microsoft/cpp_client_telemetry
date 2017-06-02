// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <IRuntimeConfig.hpp>
#include <ILogger.hpp>
#include "ContextFieldsProvider.hpp"
#include "BaseDecorator.hpp"
#include "EventPropertiesDecorator.hpp"
#include "RuntimeConfigDecorator.hpp"
#include "SemanticApiDecorators.hpp"
#include "SemanticContextDecorator.hpp"

namespace ARIASDK_NS_BEGIN {


class ILogManagerInternal;

class ARIASDK_LIBABI Logger : public ILogger
{
  public:
    Logger(std::string const& tenantToken, std::string const& source, std::string const& experimentationProject,
        ILogManagerInternal& logManager, ContextFieldsProvider* parentContext, IRuntimeConfig& runtimeConfig);
    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;
     ~Logger();

  public:
    virtual void  SetContext(const std::string& name, const char value[], PiiKind piiKind = PiiKind_None)  override;
    virtual void  SetContext(const std::string& name, const std::string& value, PiiKind piiKind = PiiKind_None) override;
    virtual void  SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None) override;
    virtual void  SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None) override;
    virtual void  SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None) override;
    virtual void  SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None) override;
    virtual void  SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None) override;
	virtual void SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }

    virtual ISemanticContext*   GetSemanticContext() const override;
    virtual void  LogAppLifecycle(AppLifecycleState state, EventProperties const& properties) override;
    virtual void  LogSession(SessionState state, const EventProperties& properties) override;
    virtual void  LogEvent(std::string const& name) override;
    virtual void  LogEvent(EventProperties const& properties) override;
    virtual void  LogFailure(std::string const& signature,
        std::string const& detail,
        std::string const& category,
        std::string const& id,
        EventProperties const& properties) override;
    virtual void  LogFailure(std::string const& signature,
        std::string const& detail,
        EventProperties const& properties) override;
    virtual void  LogPageView(std::string const& id,
        std::string const& pageName,
        std::string const& category,
        std::string const& uri,
        std::string const& referrerUri,
        EventProperties const& properties) override;
    virtual void  LogPageView(std::string const& id,
        std::string const& pageName,
        EventProperties const& properties) override;
    virtual void  LogPageAction(std::string const& pageViewId,
        ActionType actionType,
        EventProperties const& properties) override;
    virtual void  LogPageAction(PageActionData const& pageActionData,
        EventProperties const& properties) override;
    virtual void  LogSampledMetric(std::string const& name,
        double value,
        std::string const& units,
        std::string const& instanceName,
        std::string const& objectClass,
        std::string const& objectId,
        EventProperties const& properties) override;
    virtual void  LogSampledMetric(std::string const& name,
        double value,
        std::string const& units,
        EventProperties const& properties) override;
    virtual void  LogAggregatedMetric(std::string const& name,
        long duration,
        long count,
        EventProperties const& properties) override;
    virtual void  LogAggregatedMetric(AggregatedMetricData const& metricData,
        EventProperties const& properties) override;
    virtual void  LogTrace(TraceLevel level,
        std::string const& message,
        EventProperties const& properties) override;
    virtual void  LogUserState(UserState state,
        long timeToLiveInMillis,
        EventProperties const& properties) override;

  protected:
    bool applyCommonDecorators(::AriaProtocol::Record& record, EventProperties const& properties, ::Microsoft::Applications::Telemetry::EventPriority& priority);
    virtual void submit(::AriaProtocol::Record& record, ::Microsoft::Applications::Telemetry::EventPriority priority, std::uint64_t  const& policyBitFlags);
    void SetContext(const std::string& name, EventProperty prop);
  protected:
    std::mutex*              m_lockP;
    std::string*             m_tenantTokenP;
    std::string*             m_sourceP;
    ILogManagerInternal&     m_logManager;
    ContextFieldsProvider    m_context;
    IRuntimeConfig&          m_runtimeConfig;

    BaseDecorator            m_baseDecorator;
    EventPropertiesDecorator m_eventPropertiesDecorator;
    RuntimeConfigDecorator   m_runtimeConfigDecorator;
    SemanticContextDecorator m_semanticContextDecorator;
    SemanticApiDecorators    m_semanticApiDecorators;
    int64_t                  m_sessionStartTime;
    std::string*             m_sessionIdP;
};


} ARIASDK_NS_END
