// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IDecorator.hpp"
#include "api/ContextFieldsProvider.hpp"

namespace ARIASDK_NS_BEGIN {


class ARIASDK_LIBABI SemanticContextDecorator : public DecoratorBase {
  protected:
    ContextFieldsProvider const& m_provider;

  public:
    SemanticContextDecorator(ContextFieldsProvider const& provider)
      : m_provider(provider)
    {
    }
	
	SemanticContextDecorator(SemanticContextDecorator const& copy)
		: m_provider(copy.m_provider)
	{
	}

    bool decorate(::AriaProtocol::Record& record) const
    {
        m_provider.writeToRecord(record);
        return true;
    }
};


} ARIASDK_NS_END
