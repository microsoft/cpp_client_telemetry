// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include <IRuntimeConfig.hpp>

namespace ARIASDK_NS_BEGIN {


class ARIASDK_LIBABI RuntimeConfigDecorator : public DecoratorBase {
  public:
    RuntimeConfigDecorator(IRuntimeConfig& runtimeConfig, std::string const& tenantId, std::string const& experimentationProject)
      : m_runtimeConfig(runtimeConfig),
        m_tenantIdP(new std::string(tenantId)),
        m_experimentationProjectP( new std::string(experimentationProject))
    {
    }

	RuntimeConfigDecorator(RuntimeConfigDecorator const& copy)
		:m_runtimeConfig(copy.m_runtimeConfig)
	{
		m_tenantIdP = new std::string(*copy.m_tenantIdP);
		m_experimentationProjectP = new std::string(*copy.m_experimentationProjectP);
	}

	RuntimeConfigDecorator& operator=(RuntimeConfigDecorator const& copy)
	{
		m_runtimeConfig = copy.m_runtimeConfig;
		m_tenantIdP = new std::string(*copy.m_tenantIdP);
		m_experimentationProjectP = new std::string(*copy.m_experimentationProjectP);
		return *this;
	}


	~RuntimeConfigDecorator()
	{
		if (m_tenantIdP) delete m_tenantIdP;
		if (m_experimentationProjectP) delete m_experimentationProjectP;
	}

    bool decorate(::AriaProtocol::Record& record, EventPriority& priority)
    {
		UNREFERENCED_PARAMETER(priority);
        m_runtimeConfig.DecorateEvent(record.Extension, *m_experimentationProjectP, record.EventType);

   /*     EventPriority priorityOverride = m_runtimeConfig.GetEventPriority(m_tenantId, record.EventType);
        if (priorityOverride != EventPriority_Unspecified && priorityOverride != priority) {
            ARIASDK_LOG_DETAIL("Priority of event %s/%s was %sgraded from %u (%s) to %u (%s)",
                m_tenantId.c_str(), record.EventType.c_str(),
                (priorityOverride > priority) ? "up" : "down", priority, priorityToStr(priority),
                priorityOverride, priorityToStr(priorityOverride));
            priority = priorityOverride;
        }*/

        return true;
    }

  protected:
    IRuntimeConfig& m_runtimeConfig;
    std::string*     m_tenantIdP;
    std::string*     m_experimentationProjectP;
};


} ARIASDK_NS_END
