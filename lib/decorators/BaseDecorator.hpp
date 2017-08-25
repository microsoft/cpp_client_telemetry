// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include "Config.hpp"
#include <Enums.hpp>
#include <algorithm>

namespace ARIASDK_NS_BEGIN {


class BaseDecorator : public DecoratorBase {
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

    bool decorate(::AriaProtocol::CsEvent& record, ::Microsoft::Applications::Telemetry::EventPriority priority)
    {
        UNREFERENCED_PARAMETER(priority);
       
        //record.Id         = PAL::generateUuidString();
        record.time       = PAL::getUtcSystemTimeMs();
        std::string sdkVersion = PAL::getSdkVersion();
        record.ver = sdkVersion;
        if (record.baseType.empty())
        {  
            record.baseType = record.name;
        } 
      
        //if (record.data.size() == 0) {        AriaProtocol::Data temp;            record.data.push_back(temp);        }
        //setIfNotEmpty(record.data[0].properties,            EventInfo_Source,     *m_sourceP);
        //setIfNotEmpty(record.data[0].properties,            COMMONFIELDS_EVENT_TIME,       PAL::formatUtcTimestampMsAsISO8601(record.time));
        //setIfNotEmpty(record.data[0].properties,            EventInfo_InitId,     *m_initIdP);
        record.seqNum = ++m_sequenceId;
        //record.popSample = 100.00;
        //record.epoch = "epoch";

        return true;
    }

  protected:
    std::string* m_sourceP;
    std::string* m_initIdP;
    uint64_t    m_sequenceId;
};


} ARIASDK_NS_END
