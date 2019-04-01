#include "BaseDecorator.hpp"

namespace ARIASDK_NS_BEGIN {

    BaseDecorator::BaseDecorator(ILogManager& owner)
        :
        DecoratorBase(owner),
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
        record.ver = "3.0";
        if (record.baseType.empty())
        {
            record.baseType = record.name;
        }

        record.extSdk[0].seq = ++m_sequenceId;
        record.extSdk[0].epoch = m_initId;
        record.extSdk[0].libVer = PAL::getSdkVersion();
        record.extSdk[0].installId = m_owner.GetLogSessionData()->getSessionSDKUid();

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

} ARIASDK_NS_END
