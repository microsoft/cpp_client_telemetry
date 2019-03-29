// Copyright (c) Microsoft. All rights reserved.
#ifndef BASEDECORATOR_HPP
#define BASEDECORATOR_HPP

#include "Version.hpp"

#include "IDecorator.hpp"
#include "Enums.hpp"
#include "ILogManager.hpp"

#include <algorithm>


namespace ARIASDK_NS_BEGIN
{

    class BaseDecorator : public DecoratorBase
    {

    public:
        BaseDecorator(ILogManager& owner);
        virtual ~BaseDecorator() {};
        bool decorate(::CsProtocol::Record& record);

    protected:
        std::string             m_source;
        std::string             m_initId;
        uint64_t                m_sequenceId;
    };

} ARIASDK_NS_END

#endif
