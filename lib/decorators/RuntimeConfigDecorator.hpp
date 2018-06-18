// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include "api/IRuntimeConfig.hpp"
#include "ILogManager.hpp"

namespace ARIASDK_NS_BEGIN {

    class RuntimeConfigDecorator : public DecoratorBase {
    public:

        RuntimeConfigDecorator(ILogManager& owner, IRuntimeConfig& runtimeConfig, std::string const& tenantId, std::string const& experimentationProject)
            :
            DecoratorBase(owner),
            m_config(runtimeConfig),
            m_tenantIdP(new std::string(tenantId)),
            m_experimentationProjectP(new std::string(experimentationProject))
        {
        }

/*
        RuntimeConfigDecorator(RuntimeConfigDecorator const& copy)
            :m_config(copy.m_config)
        {
            m_tenantIdP = new std::string(*copy.m_tenantIdP);
            m_experimentationProjectP = new std::string(*copy.m_experimentationProjectP);
        }

        RuntimeConfigDecorator& operator=(RuntimeConfigDecorator const& copy)
        {
            m_config = copy.m_config;
            m_tenantIdP = new std::string(*copy.m_tenantIdP);
            m_experimentationProjectP = new std::string(*copy.m_experimentationProjectP);
            return *this;
        }
 */

        ~RuntimeConfigDecorator()
        {
            if (m_tenantIdP) delete m_tenantIdP;
            if (m_experimentationProjectP) delete m_experimentationProjectP;
        }

        bool decorate(::AriaProtocol::Record& record)
        {
            UNREFERENCED_PARAMETER(record);

            return true;
        }

    protected:
        IRuntimeConfig & m_config;
        std::string*     m_tenantIdP;
        std::string*     m_experimentationProjectP;
    };


} ARIASDK_NS_END
