//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "BandwidthController_ResourceManager.hpp"
#include "ResourceManager/RMPublic_BitStream.hpp"

namespace MAT_NS_BEGIN {


BandwidthController_ResourceManager::BandwidthController_ResourceManager(::resource_manager2::ResourceManagerPtr const& rm)
  : m_rm(rm)
{
    if (m_rm.isNull()) {
        return;
    }

    // ResourceManager is already informed about the actual telemetry upload
    // traffic by HttpStack through RM class TcpRequest. All such HttpStack
    // requests are also correctly labeled for throttling. So this class
    // should not report any actual traffic to RM, because that would result
    // in it being counted twice towards the total bandwidth.
    //
    // But a connection and a stream need to be created nevertheless in order
    // to get proposed bandwidth recommendations for a dummy outbound stream
    // of type "DataRV".

    m_conn = m_rm->createConnection(true, 1122334455, false, NULL);
    if (m_conn.isNull()) {
        return;
    }

    m_stream = m_rm->createOutboundStream(resource_manager2::StreamConfig(), resource_manager2::DataRV);
    if (m_stream.isNull()) {
        return;
    }

    m_conn->attachStream(m_stream->getTag());
}

BandwidthController_ResourceManager::~BandwidthController_ResourceManager()
{
    if (!m_stream.isNull()) {
        m_conn->detachStream(m_stream->getTag());
    }
}

unsigned BandwidthController_ResourceManager::GetProposedBandwidthBps()
{
    if (m_stream.isNull()) {
        return ~0;
    }

    resource_manager2::StreamStatus status;
    m_stream->getStatus(status);
    return status.proposedBandwidthBps;
}


} MAT_NS_END

