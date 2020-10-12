//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
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

