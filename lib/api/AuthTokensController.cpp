// Copyright (c) Microsoft. All rights reserved.
#include "pal/PAL.hpp"
#include "AuthTokensController.hpp"
#include "CommonLogManagerInternal.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN {

ARIASDK_LOG_INST_COMPONENT_CLASS(AuthTokensController, "EventsSDK.AuthTokensController", "Events telemetry client - AuthTokensController class");

AuthTokensController::AuthTokensController()
{
    ARIASDK_LOG_DETAIL("New AuthTokensController instance");
}

AuthTokensController::~AuthTokensController()
{
    ARIASDK_LOG_INFO("destructor");
}

EVTStatus  AuthTokensController::SetTicketToken(TicketType type, char const* tokenValue)
{
    if (nullptr != tokenValue)
    {        
        if (type == TicketType::TicketType_MSA_Device ||
            type == TicketType::TicketType_AAD ||
            type == TicketType::TicketType_XAuth_Device)
        {
            m_deviceTokens[type] = std::string(tokenValue);
        }
        else
        {
            m_tickets.push_back(TICKETS_PREPAND_STRING + std::to_string(type));
            m_userTokens[type] = std::string(tokenValue);
        }
        return EVTStatus::EVTStatus_OK;
    }
    return EVTStatus::EVTStatus_Fail;
}

std::vector<std::string>&  AuthTokensController::GetTickets()
{
    return m_tickets;
}

std::map<TicketType, std::string>&  AuthTokensController::GetDeviceTokens()
{
    return m_deviceTokens;
}

std::map<TicketType, std::string>&  AuthTokensController::GetUserTokens()
{
    return m_userTokens;
}
}}} // namespace Microsoft::Applications::Events 
