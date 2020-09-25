// Copyright (c) Microsoft. All rights reserved.
#ifndef SEMANTICCONTEXTDECORATOR_HPP
#define SEMANTICCONTEXTDECORATOR_HPP

#include "IDecorator.hpp"
#include "api/ContextFieldsProvider.hpp"

namespace MAT_NS_BEGIN
{

    class SemanticContextDecorator : public IDecorator
    {

    protected:
        ILogManager&           m_owner;
        ContextFieldsProvider& provider;

    public:
        SemanticContextDecorator(ILogManager& owner) :
            m_owner(owner),
            provider(static_cast<ContextFieldsProvider&>(owner.GetSemanticContext()))
        {
        }

        SemanticContextDecorator(ILogManager& owner, ContextFieldsProvider& context) :
            m_owner(owner),
            provider(context)
        {
        }

        bool decorate(::CsProtocol::Record& record) override
        {
            return decorate(record, false);
        }

        bool decorate(::CsProtocol::Record& record, bool commonOnly)
        {
            provider.writeToRecord(record, commonOnly);
            return true;
        }

    };


} MAT_NS_END
#endif
