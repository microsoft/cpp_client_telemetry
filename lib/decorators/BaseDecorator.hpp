// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Version.hpp"

#include "IDecorator.hpp"

#include "Config.hpp"
#include <Enums.hpp>
#include <algorithm>

#include "ILogManager.hpp"

namespace ARIASDK_NS_BEGIN
{

    class BaseDecorator : public DecoratorBase
    {

    public:
        BaseDecorator(ILogManager& owner);
        virtual ~BaseDecorator() {};
        bool decorate(::AriaProtocol::Record& record);

    protected:
        std::string             m_source;
        std::string             m_initId;
        uint64_t                m_sequenceId;
    };

} ARIASDK_NS_END
