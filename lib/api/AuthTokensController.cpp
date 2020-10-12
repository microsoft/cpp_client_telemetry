//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "pal/PAL.hpp"
#include "AuthTokensController.hpp"
#include "ILogManager.hpp"
#include "utils/Utils.hpp"

namespace MAT_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(AuthTokensController, "EventsSDK.AuthTokensController", "Events telemetry client - AuthTokensController class");

    AuthTokensController::AuthTokensController()
        :m_IsStrictModeEnabled(false)
    {
        LOG_TRACE("New AuthTokensController instance");
    }

    AuthTokensController::~AuthTokensController()
    {
        LOG_TRACE("destructor");
    }

    status_t  AuthTokensController::SetTicketToken(TicketType type, char const* tokenValue)
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
                m_tickets.push_back(TICKETS_PREPEND_STRING + std::to_string(type));
                m_userTokens[type] = std::string(tokenValue);
            }
            return STATUS_SUCCESS;
        }
        return STATUS_EFAIL;
    }

    status_t  AuthTokensController::Clear()
    {
        m_deviceTokens.clear();
        m_userTokens.clear();
        m_tickets.clear();
        return STATUS_SUCCESS;
    }

    status_t  AuthTokensController::SetStrictMode(bool value)
    {
        m_IsStrictModeEnabled = value;
        return STATUS_EFAIL;
    }

    bool  AuthTokensController::GetStrictMode()
    {
        return m_IsStrictModeEnabled;
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

} MAT_NS_END

