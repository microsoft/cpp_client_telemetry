// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include <IRuntimeConfig.hpp>

namespace ARIASDK_NS_BEGIN {


class RuntimeConfigDecorator : public DecoratorBase {
  public:
    RuntimeConfigDecorator(IRuntimeConfig* runtimeConfig, std::string const& tenantId, std::string const& experimentationProject)
      : m_runtimeConfigP(runtimeConfig),
        m_tenantIdP(new std::string(tenantId)),
        m_experimentationProjectP( new std::string(experimentationProject))
    {
    }

	RuntimeConfigDecorator(RuntimeConfigDecorator const& copy)
		:m_runtimeConfigP(copy.m_runtimeConfigP)
	{
		m_tenantIdP = new std::string(*copy.m_tenantIdP);
		m_experimentationProjectP = new std::string(*copy.m_experimentationProjectP);
	}

	RuntimeConfigDecorator& operator=(RuntimeConfigDecorator const& copy)
	{
		m_runtimeConfigP = copy.m_runtimeConfigP;
		m_tenantIdP = new std::string(*copy.m_tenantIdP);
		m_experimentationProjectP = new std::string(*copy.m_experimentationProjectP);
		return *this;
	}


	~RuntimeConfigDecorator()
	{
		if (m_tenantIdP) delete m_tenantIdP;
		if (m_experimentationProjectP) delete m_experimentationProjectP;
	}

    bool decorate(::AriaProtocol::CsEvent& record)
    {
        UNREFERENCED_PARAMETER(record);
        //Test use runtimeconfig decorate to verify an event is logged. I just passed tags, nothing gets changed in runtime config decorators.
        //m_runtimeConfig.DecorateEvent(record.tags, *m_experimentationProjectP, record.baseType);

   /*     EventLatency priorityOverride = m_runtimeConfigP->GetEventLatency(m_tenantId, record.baseType);
        if (priorityOverride != EventPriority_Unspecified && priorityOverride != priority) {
            ARIASDK_LOG_DETAIL("Priority of event %s/%s was %sgraded from %u (%s) to %u (%s)",
                m_tenantId.c_str(), record.baseType.c_str(),
                (priorityOverride > priority) ? "up" : "down", priority, priorityToStr(priority),
                priorityOverride, priorityToStr(priorityOverride));
            priority = priorityOverride;
        }*/

        return true;
    }

  protected:
    IRuntimeConfig*  m_runtimeConfigP;
    std::string*     m_tenantIdP;
    std::string*     m_experimentationProjectP;
};


} ARIASDK_NS_END
