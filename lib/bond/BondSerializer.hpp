//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "system/Contexts.hpp"
#include "system/Route.hpp"

namespace MAT_NS_BEGIN {


class BondSerializer {
  protected:
    bool handleSerialize(IncomingEventContextPtr const& ctx);

  public:
    RoutePassThrough<BondSerializer, IncomingEventContextPtr const&> serialize{this, &BondSerializer::handleSerialize};
};


} MAT_NS_END

