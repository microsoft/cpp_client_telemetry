// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include "api/CommonLogManagerInternal.hpp"
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

    bool decorate(::AriaProtocol::CsEvent& record)
    {
        if (record.extSdk.size() == 0)
        {
            ::AriaProtocol::Sdk sdk;
            record.extSdk.push_back(sdk);
        }
      
        record.time       = PAL::getUtcSystemTimeinTicks();       
        record.ver = "3.0";        
        if (record.baseType.empty())
        {  
            record.baseType = record.name;
        }     

        record.extSdk[0].seq = ++m_sequenceId;        
        record.extSdk[0].epoch = *m_initIdP;
        std::string sdkVersion = PAL::getSdkVersion();
        record.extSdk[0].libVer = sdkVersion;
        LogSessionData* logSessionData = CommonLogManagerInternal::GetLogSessionData();
        if (logSessionData)
        {
            record.extSdk[0].installId = CommonLogManagerInternal::GetLogSessionData()->getSessionSDKUid();
        }

        //set Tickets
        if (CommonLogManagerInternal::GetAuthTokensController()->GetTickets().size() > 0)
        {
            if (record.extProtocol.size() == 0)
            {
                ::AriaProtocol::Protocol temp;
                record.extProtocol.push_back(temp);
            }
            if (record.extProtocol[0].ticketKeys.size() == 0)
            {
                std::vector<std::string> temp;
                record.extProtocol[0].ticketKeys.push_back(temp);
            }
            for (auto ticket : CommonLogManagerInternal::GetAuthTokensController()->GetTickets())
            {
                record.extProtocol[0].ticketKeys[0].push_back(ticket);
            }           
        }
    

        return true;
    }

  protected:
    std::string* m_sourceP;
    std::string* m_initIdP;
    uint64_t    m_sequenceId;
};


} ARIASDK_NS_END
