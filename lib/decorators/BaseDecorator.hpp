// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include "Config.hpp"
#include <Enums.hpp>
#include <algorithm>

namespace ARIASDK_NS_BEGIN {


class ARIASDK_LIBABI BaseDecorator : public DecoratorBase {
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

        
        setIfNotEmpty(record.Extension,            EventInfo_Name,       record.EventType);
        setIfNotEmpty(record.Extension,            EventInfo_Source,     *m_sourceP);
        setIfNotEmpty(record.Extension,            COMMONFIELDS_EVENT_TIME,       PAL::formatUtcTimestampMsAsISO8601(record.Timestamp));
        setIfNotEmpty(record.Extension,            EventInfo_InitId,     *m_initIdP);
        setInt64Value(record.TypedExtensionInt64,  EventInfo_Sequence,   ++m_sequenceId);
        setIfNotEmpty(record.Extension,            COMMONFIELDS_EVENT_SDKVERSION, PAL::getSdkVersion());

        setInt64Value(record.TypedExtensionInt64, "eventpriority",  std::max<int>(0, priority));

        return true;
    }

  protected:
    std::string* m_sourceP;
    std::string* m_initIdP;
    uint64_t    m_sequenceId;
};


} ARIASDK_NS_END
