// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Version.hpp"
#include "Enums.hpp"
#include "pal/PAL.hpp"
#include "IAuthTokensController.hpp"
#include <map>
#include <vector>

// *INDENT-OFF*
namespace ARIASDK_NS_BEGIN
{
    /// <summary>
    /// This class is used to manage the Auth Tokens
    /// </summary>
    class AuthTokensController : public IAuthTokensController
    {
    public:

        AuthTokensController();
        /// <summary>
        /// Destroy the telemetry logging system instance. Calls `FlushAndTeardown()` implicitly.
        /// </summary>
        virtual ~AuthTokensController();

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual status_t  SetTicketToken(TicketType type, char const* tokenValue);

        /// <summary>
        /// Clears all tokens.
        /// </summary>
        virtual status_t  Clear();

        /// <summary>
        /// sets strict mode for application( all tokens in that app).
        /// </summary>
        virtual status_t  SetStrictMode(bool value);

        /// <summary>
        /// gets strict mode for application.
        /// </summary>
        virtual bool  GetStrictMode();

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual std::vector<std::string>&  GetTickets();

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual std::map<TicketType, std::string>&  GetDeviceTokens();

        /// <summary>
        /// Set the Auth ticket.
        /// </summary>
        /// <param name="type">Ticket type</param>
        /// <param name="ticketValue">Ticketvalue</param>
        virtual std::map<TicketType, std::string>&  GetUserTokens();

    private:
        ARIASDK_LOG_DECL_COMPONENT_CLASS();
        std::map<TicketType, std::string> m_deviceTokens;
        std::map<TicketType, std::string> m_userTokens;
        std::vector<std::string> m_tickets;
        bool m_IsStrictModeEnabled;
    };


} ARIASDK_NS_END
