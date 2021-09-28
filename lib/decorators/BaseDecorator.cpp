//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#include "BaseDecorator.hpp"

namespace MAT_NS_BEGIN {

    BaseDecorator::BaseDecorator(ILogManager& owner)
        :
        m_owner(owner),
        // TODO: populate m_source
        m_initId(PAL::generateUuidString()),
        m_sequenceId(0)
    {
    }

    /// <summary>
    /// Decorates the specified record.
    /// </summary>
    /// <param name="record">The record.</param>
    /// <returns>true if successful</returns>
    bool BaseDecorator::decorate(::CsProtocol::Record& record)
    {
        if (record.extSdk.size() == 0)
        {
            ::CsProtocol::Sdk sdk;
            record.extSdk.push_back(sdk);
        }

        record.time = PAL::getUtcSystemTimeinTicks();
        record.ver = ::CsProtocol::CS_VER_STRING;
        if (record.baseType.empty())
        {
            record.baseType = record.name;
        }

        record.extSdk[0].seq = ++m_sequenceId;
        record.extSdk[0].epoch = m_initId;
        // Backward compat note:
        // - CS3 named this field libVer.
        // - CS4 moved the libVer from #0 to post #5. And renamed pos #0 from libVer to ver
        // This creates a ton of confusion, but we keep things where they were before.
#ifdef HAVE_CS4
        record.extSdk[0].ver = PAL::getSdkVersion();
#else
        record.extSdk[0].libVer = PAL::getSdkVersion();
#endif
        auto sessionData = m_owner.GetLogSessionData();
        if (sessionData)
        {
            record.extSdk[0].installId = sessionData->getSessionSDKUid();
        }

        //set Tickets
        if ((m_owner.GetAuthTokensController()) &&
            (m_owner.GetAuthTokensController()->GetTickets().size() > 0))
        {
            auto tokensController = m_owner.GetAuthTokensController();
            if (record.extProtocol.size() == 0)
            {
                ::CsProtocol::Protocol temp;
                record.extProtocol.push_back(temp);
            }
            if (record.extProtocol[0].ticketKeys.size() == 0)
            {
                std::vector<std::string> temp;
                record.extProtocol[0].ticketKeys.push_back(temp);
            }
            for (const auto& ticket : tokensController->GetTickets())
            {
                record.extProtocol[0].ticketKeys[0].push_back(ticket);
            }
        }
        return true;
    }

} MAT_NS_END

