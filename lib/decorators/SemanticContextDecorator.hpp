// Copyright (c) Microsoft. All rights reserved.
#ifndef SEMANTICCONTEXTDECORATOR_HPP
#define SEMANTICCONTEXTDECORATOR_HPP

#include "IDecorator.hpp"
#include "api/ContextFieldsProvider.hpp"

namespace ARIASDK_NS_BEGIN
{

    class SemanticContextDecorator : public DecoratorBase
    {

    protected:
        ContextFieldsProvider& provider;

    public:
        SemanticContextDecorator(ILogManager& owner) :
            DecoratorBase(owner),
            provider(static_cast<ContextFieldsProvider&>(m_owner.GetSemanticContext()))
        {
        }

        SemanticContextDecorator(ILogManager& owner, ContextFieldsProvider& context) :
            DecoratorBase(owner),
            provider(context)
        {
        }

        bool decorate(::CsProtocol::Record& record, bool commonOnly = false)
        {
            provider.writeToRecord(record, commonOnly);
            return true;
        }

    };


} ARIASDK_NS_END
#endif
