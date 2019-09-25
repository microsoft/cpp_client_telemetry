// Copyright (c) Microsoft. All rights reserved.
#ifndef SEMANTICCONTEXTDECORATOR_HPP
#define SEMANTICCONTEXTDECORATOR_HPP

#include "IDecorator.hpp"
#include "api/ContextFieldsProvider.hpp"

namespace ARIASDK_NS_BEGIN
{

    class SemanticContextDecorator : public IDecorator
    {

    protected:
        ILogManager&           m_owner;
        ContextFieldsProvider& provider;

    public:
        SemanticContextDecorator(ILogManager& owner) :
            IDecorator(),
            m_owner(owner),
            provider(static_cast<ContextFieldsProvider&>(owner.GetSemanticContext()))
        {
        }

        SemanticContextDecorator(ILogManager& owner, ContextFieldsProvider& context) :
            IDecorator(),
            m_owner(owner),
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
