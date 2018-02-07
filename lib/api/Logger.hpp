// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <IRuntimeConfig.hpp>
#include <ILogger.hpp>
#include "ContextFieldsProvider.hpp"
#include "decorators/BaseDecorator.hpp"
#include "decorators/EventPropertiesDecorator.hpp"
#include "decorators/RuntimeConfigDecorator.hpp"
#include "decorators/SemanticApiDecorators.hpp"
#include "decorators/SemanticContextDecorator.hpp"

namespace ARIASDK_NS_BEGIN {


class ILogManagerInternal;

class ARIASDK_LIBABI Logger : public ILogger
{
  public:
    Logger(std::string const& tenantToken, std::string const& source, std::string const& experimentationProject,
        ILogManagerInternal* logManager, ContextFieldsProvider* parentContext, IRuntimeConfig* runtimeConfig);
    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;
     ~Logger();

  public:
    virtual void SetContext(const std::string& name, const char value[], PiiKind piiKind = PiiKind_None)  override;
    virtual void SetContext(const std::string& name, const std::string& value, PiiKind piiKind = PiiKind_None) override;
    virtual void SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None) override;
    virtual void SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None) override;
    virtual void SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None) override;
    virtual void SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None) override;
    virtual void SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None) override;
	virtual void SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }
	virtual void SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) override { SetContext(name, (int64_t)value, piiKind); }

    virtual ISemanticContext*   GetSemanticContext() const override;
    virtual void  LogEvent(std::string const& name) override;
    virtual void  LogEvent(EventProperties const& properties) override;

  protected:
    bool applyCommonDecorators(::AriaProtocol::CsEvent& record, EventProperties const& properties, ::Microsoft::Applications::Events ::EventLatency& latency);
    virtual void submit(::AriaProtocol::CsEvent& record, 
                        ::Microsoft::Applications::Events ::EventLatency latency,
                        ::Microsoft::Applications::Events ::EventPersistence persistence,
                        std::uint64_t  const& policyBitFlags);
    void SetContext(const std::string& name, EventProperty prop);
    std::mutex*              m_lockP;
    std::string*             m_tenantTokenP;
    std::string*             m_sourceP;
    ILogManagerInternal*             m_logManagerP;
    ContextFieldsProvider*   m_context;

    BaseDecorator*            m_baseDecorator;
    EventPropertiesDecorator* m_eventPropertiesDecorator;
    RuntimeConfigDecorator*   m_runtimeConfigDecorator;
    SemanticContextDecorator* m_semanticContextDecorator;
    SemanticApiDecorators*    m_semanticApiDecorators;
    int64_t                  m_sessionStartTime;
    std::string*             m_sessionIdP;
};


} ARIASDK_NS_END
