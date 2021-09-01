//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "aria/IBandwidthController.hpp"
#include "ResourceManager/ResourceManagerPublic.hpp"

namespace MAT_NS_BEGIN {


class BandwidthController_ResourceManager : public IBandwidthController
{
  public:
    BandwidthController_ResourceManager(resource_manager2::ResourceManagerPtr const& rm);
    virtual ~BandwidthController_ResourceManager();
    virtual unsigned GetProposedBandwidthBps() override;

  protected:
    resource_manager2::ResourceManagerPtr m_rm;
    resource_manager2::ConnectionPtr      m_conn;
    resource_manager2::BitStreamPtr       m_stream;
};


} MAT_NS_END

