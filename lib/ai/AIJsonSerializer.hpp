// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "system/Contexts.hpp"
#include "system/Route.hpp"

namespace ARIASDK_NS_BEGIN {

class AIJsonSerializer {
  protected:
    bool handleSerialize(IncomingEventContextPtr const& ctx);

  public:
    RoutePassThrough<AIJsonSerializer, IncomingEventContextPtr const&> serialize{this, &AIJsonSerializer::handleSerialize};
};

} ARIASDK_NS_END
