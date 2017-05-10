// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include <aria/Enums.hpp>
#include <algorithm>

namespace ARIASDK_NS_BEGIN {


class ARIASDK_LIBABI BaseDecorator : public IDecorator {
  public:
	BaseDecorator()
		  : m_sourceP(new std::string("")),
		  m_initIdP(new std::string(PAL::generateUuidString())),
		  m_sequenceId(0)
	{
	}
    BaseDecorator(std::string const& source)
      : m_sourceP(new std::string(source)),
        m_initIdP(new std::string(PAL::generateUuidString())),
        m_sequenceId(0)
    {
    }

	BaseDecorator(BaseDecorator const& copy)
	{
		m_sourceP = new std::string(*copy.m_sourceP);
		m_initIdP = new std::string(*copy.m_initIdP);
	}

	BaseDecorator& operator=(BaseDecorator const& copy)
	{
		m_sourceP = new std::string(*copy.m_sourceP);
		m_initIdP = new std::string(*copy.m_initIdP);
		return *this;
	}

	~BaseDecorator()
	{
		if (m_sourceP) delete m_sourceP;
		if (m_initIdP) delete m_initIdP;
	}

    bool decorate(::AriaProtocol::Record& record, ::Microsoft::Applications::Telemetry::EventPriority priority)
    {
        record.Id         = PAL::generateUuidString();
        record.Timestamp  = PAL::getUtcSystemTimeMs();
        if (record.Type.empty()) {
            record.Type = "Custom";
        }
        record.RecordType = AriaProtocol::RecordType::Event;

        setIfNotEmpty(record.Extension, "EventInfo.Name",       record.EventType);
        setIfNotEmpty(record.Extension, "EventInfo.Source",     *m_sourceP);
        setIfNotEmpty(record.Extension, "EventInfo.Time",       PAL::formatUtcTimestampMsAsISO8601(record.Timestamp));
        setIfNotEmpty(record.Extension, "EventInfo.InitId",     *m_initIdP);
        setOtherValue(record.Extension, "EventInfo.Sequence",   ++m_sequenceId);
        setIfNotEmpty(record.Extension, "EventInfo.SdkVersion", PAL::getSdkVersion());

        setOtherValue(record.Extension, "eventpriority",  std::max<int>(0, priority));

        return true;
    }

  protected:
    std::string* m_sourceP;
    std::string* m_initIdP;
    uint64_t    m_sequenceId;
};


} ARIASDK_NS_END
