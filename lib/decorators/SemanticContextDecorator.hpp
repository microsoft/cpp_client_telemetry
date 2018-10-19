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

        bool decorate(::AriaProtocol::Record& record)
        {
            ContextFieldsProvider& provider = static_cast<ContextFieldsProvider&>(m_owner.GetSemanticContext());
            provider.writeToRecord(record);
            return true;
        }

    };


} ARIASDK_NS_END
