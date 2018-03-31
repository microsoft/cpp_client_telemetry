// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include "api/ContextFieldsProvider.hpp"

namespace ARIASDK_NS_BEGIN {

    class SemanticContextDecorator : public DecoratorBase {

    public:
        SemanticContextDecorator(ILogManager& owner):
            DecoratorBase(owner)
        {
        }

#if 0
        SemanticContextDecorator(SemanticContextDecorator const& copy)
            : m_provider(copy.m_provider)
        {
        }
#endif
        bool decorate(::AriaProtocol::Record& record)
        {
            ContextFieldsProvider& provider = dynamic_cast<ContextFieldsProvider&>(m_owner.GetSemanticContext());
            provider.writeToRecord(record);
            return true;
        }

    };


} ARIASDK_NS_END
